#include <file_processor.hpp>

#include <file_hash.hpp>
#include <file_io.hpp>
#include <worker_pool.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <boost/thread/barrier.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdio>

FileProcessor::FileProcessor()
    :m_cancelProcessing(false)
{}

FileProcessor::~FileProcessor()
{
    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }
}

void FileProcessor::startProcessing(BlimpDB::SnapshotId snapshot_id, std::vector<FileInfo>&& files, std::unique_ptr<BlimpDB>&& blimpdb)
{
    GHULBUS_PRECONDITION(!m_dbReturnChannel);
    m_dbReturnChannel = std::move(blimpdb);
    m_filesToProcess = std::move(files);
    m_cancelProcessing.store(false);
    m_processingThread = std::thread([this, snapshot_id]() {
        FileIO fio;
        FileHasher hasher(HashType::SHA_256);
        WorkerPool pool(1);
        boost::barrier barrier(2);
        std::size_t file_index = 0;
        for (auto const& f : m_filesToProcess) {
            // read file chunk
            emit processingUpdateNewFile(file_index, f.size);
            GHULBUS_LOG(Debug, "Processing file " << f.path.string());
            fio.startReading(f.path);
            std::size_t bytes_read = 0;
            hasher.restart();
            while (fio.hasMoreChunks()) {
                FileChunk const& c = fio.getNextChunk();
                bytes_read += c.getUsedSize();
                // calculate checksum (async)
                pool.schedule([&hasher, &c, &barrier]() { hasher.addData(c); barrier.wait(); });
                // save to storage
                // @Todo
                // join checksum
                barrier.wait();
                // update db
                // @Todo
                emit processingUpdateFileProgress(bytes_read);
                if (m_cancelProcessing.load()) { emit processingCanceled(); return; }
            }
            GHULBUS_LOG(Debug, "Calculated hash for " << f.path << " is " << to_string(hasher.getHash()));
            if (m_cancelProcessing.load()) { emit processingCanceled(); return; }
            ++file_index;
        }
        emit processingCompleted();
    });
}

void FileProcessor::cancelProcessing()
{
    m_cancelProcessing.store(true);
}

std::unique_ptr<BlimpDB> FileProcessor::joinProcessing()
{
    m_processingThread.join();
    std::unique_ptr<BlimpDB> ret;
    swap(ret, m_dbReturnChannel);
    return ret;
}
