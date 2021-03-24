#include <file_processor.hpp>

#include <file_hash.hpp>
#include <file_io.hpp>
#include <processing_pipeline.hpp>
#include <storage_location.hpp>
#include <worker_pool.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <boost/filesystem/path.hpp>

#include <cstdio>
#include <vector>

FileProcessor::FileProcessor()
    :m_cancelProcessing(false), m_processingPipeline(std::make_unique<ProcessingPipeline>())
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
    m_processingThread = std::thread([this, snapshot_id, &blimpdb = *m_dbReturnChannel]() {
        FileIO fio;
        FileHasher hasher(HashType::SHA_256);
        WorkerPool pool(1);
        std::size_t file_index = 0;
        std::vector<BlimpDB::FileElementId> snapshot_contents;
        blimpdb.startExternalSync();
        for (auto const& f : m_filesToProcess) {
            try {
                // read file chunk
                emit processingUpdateNewFile(file_index, f.size);
                GHULBUS_LOG(Debug, "Processing file " << f.path.string());
                fio.startReading(f.path);
                std::size_t bytes_read = 0;
                hasher.restart();
                while (fio.hasMoreChunks()) {
                    FileChunk const& c = fio.getNextChunk();
                    bytes_read += c.getUsedSize();
                    hasher.addData(c);
                    emit processingUpdateHashProgress(bytes_read);
                    if (m_cancelProcessing.load()) { emit processingCanceled(); return; }
                }
                Hash const hash = hasher.getHash();
                GHULBUS_LOG(Debug, "Calculated hash for " << f.path << " is " << to_string(hash));
                std::size_t const filesize = bytes_read;
                if (filesize != f.size) {
                    /// @todo deal with changing files
                    GHULBUS_LOG(Warning, "File size for " << f.path << " changed during processing.");
                }
                emit processingUpdateHashCompleted(file_index, filesize);
                auto const [file_element, content_id, insertion_status] = blimpdb.newFileContent(f, hash, false);
                snapshot_contents.push_back(file_element);
                if (insertion_status == BlimpDB::FileContentInsertion::CreatedNew) {
                    auto transaction = m_processingPipeline->startNewContentTransaction(hash);
                    fio.startReading(f.path);
                    bytes_read = 0;
                    while (fio.hasMoreChunks()) {
                        FileChunk const& c = fio.getNextChunk();
                        transaction.addFileChunk(c);
                        bytes_read += c.getUsedSize();
                        emit processingUpdateFileProgress(bytes_read);
                        if (m_cancelProcessing.load()) { emit processingCanceled(); return; }
                    }
                    std::vector<StorageLocation> const storage_locations =
                        m_processingPipeline->commitTransaction(std::move(transaction));
                    blimpdb.newStorageElement(content_id, storage_locations, false);
                }
                if (m_cancelProcessing.load()) { emit processingCanceled(); return; }
            } catch (std::exception& e) {
                GHULBUS_LOG(Error, "Unable to open file " << f.path << ": " << e.what());
            }
            ++file_index;
        }
        GHULBUS_LOG(Debug, "Adding " << snapshot_contents.size() << " elements as snapshot contents");
        blimpdb.addSnapshotContents(snapshot_id, snapshot_contents, false);
        blimpdb.commitExternalSync();
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
