#include <file_processor.hpp>

#include <file_io.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <boost/filesystem/path.hpp>

#include <cstdio>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

FileProcessor::FileProcessor()
    :m_cancelProcessing(false)
{}

FileProcessor::~FileProcessor()
{}

void FileProcessor::startProcessing(BlimpDB::SnapshotId snapshot_id, std::vector<FileInfo>&& files, std::unique_ptr<BlimpDB>&& blimpdb)
{
    GHULBUS_PRECONDITION(!m_dbReturnChannel);
    m_dbReturnChannel = std::move(blimpdb);
    m_filesToProcess = std::move(files);
    m_cancelProcessing = false;
    m_processingThread = std::thread([this, snapshot_id]() {
        FileIO fio;
        std::size_t file_index = 0;
        for (auto const& f : m_filesToProcess) {
            // read file chunk
            emit processingUpdateNewFile(file_index, f.size);
            GHULBUS_LOG(Debug, "Processing file " << f.path.string());
            fio.startReading(f.path);
            std::size_t bytes_read = 0;
            while (fio.hasMoreChunks()) {
                FileChunk const& c = fio.getNextChunk();
                bytes_read += c.getUsedSize();
                // calculate checksum (async)
                // save to storage
                // join checksum
                // update db
                emit processingUpdateFileProgress(bytes_read);
            }
            ++file_index;
        }
        emit processingCompleted();
    });
}

void FileProcessor::cancelProcessing()
{
    std::lock_guard<std::mutex> lk(m_mtx);
    m_cancelProcessing = true;
}

std::unique_ptr<BlimpDB> FileProcessor::joinProcessing()
{
    m_processingThread.join();
    std::unique_ptr<BlimpDB> ret;
    swap(ret, m_dbReturnChannel);
    return ret;
}
