#include <file_scanner.hpp>

#include <exceptions.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>

#include <sha.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>

FileScanner::FileScanner()
{
}

FileScanner::~FileScanner()
{
    if(m_scanThread.joinable()) {
        m_scanThread.join();
    }
}

void FileScanner::addFilesForIndexing(std::vector<std::string> const& files_to_add)
{
    GHULBUS_LOG(Debug, "Adding " << files_to_add.size() << " item(s) for indexing.");
    std::lock_guard<std::mutex> lk(m_mtx);
    m_filesToIndex.insert(begin(m_filesToIndex), begin(files_to_add), end(files_to_add));
}

void FileScanner::startScanning(std::unique_ptr<BlimpDB> blimpdb)
{
    GHULBUS_PRECONDITION(!m_scanThread.joinable());
    GHULBUS_ASSERT(!m_dbReturnChannel);
    m_cancelScanning.store(false);
    m_scanThread = std::thread([this, blimpdb_ptr = std::move(blimpdb)]() mutable {
        BlimpDB* blimpdb = blimpdb_ptr.get();
        auto finalizer_dbReturn = Ghulbus::finally([this, blimpdb_ptr = std::move(blimpdb_ptr)]() mutable {
            m_dbReturnChannel = std::move(blimpdb_ptr);
        });
        std::vector<std::string> files_to_process;
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            files_to_process.reserve(m_filesToIndex.size());
            std::move(begin(m_filesToIndex), end(m_filesToIndex), std::back_inserter(files_to_process));
            m_filesToIndex.clear();
        }
        GHULBUS_LOG(Debug, "Indexing " << files_to_process.size() << " item(s) for scanning.");
        m_timings.indexingStart = std::chrono::steady_clock::now();
        for(auto const& f : files_to_process) {
            indexFilesRecursively(f);
            if(m_cancelScanning) {
                GHULBUS_LOG(Debug, "Aborting scanning due to cancel request.");
                return;
            }
        }
        m_timings.indexingFinished = std::chrono::steady_clock::now();
        auto const indexingDuration = m_timings.indexingFinished - m_timings.indexingStart;
        auto const indexingDurationSeconds = std::chrono::duration_cast<std::chrono::seconds>(indexingDuration);
        GHULBUS_LOG(Info, "Indexing complete after " << indexingDurationSeconds.count() << " seconds."
                           " Found " << m_fileIndexList.size() << " file(s).");
        emit indexingCompleted(m_fileIndexList.size());

        m_timings.indexDiffComputationStart = std::chrono::steady_clock::now();
        m_fileDiff = blimpdb->compareFileIndex(m_fileIndexList);
        m_timings.indexDiffComputationFinished = std::chrono::steady_clock::now();
        auto const indexDiffDuration = m_timings.indexDiffComputationFinished - m_timings.indexDiffComputationStart;
        auto const indexDiffDurationMsecs = std::chrono::duration_cast<std::chrono::milliseconds>(indexDiffDuration);
        GHULBUS_LOG(Info, "Index diffing complete after " << indexDiffDurationMsecs.count() << " milliseconds.");
        emit indexDiffCompleted();
    });
}

void FileScanner::indexFilesRecursively(boost::filesystem::path const& file_to_scan)
{
    if(m_cancelScanning) { return; }
    boost::system::error_code ec;
    auto const status = boost::filesystem::status(file_to_scan, ec);
    if(!ec) {
        if(boost::filesystem::is_directory(status)) {
            for(auto const& f : boost::filesystem::directory_iterator(file_to_scan, ec)) {
                indexFilesRecursively(f);
            }
        } else if(boost::filesystem::is_regular_file(status)) {
            FileInfo info;
            info.path = file_to_scan;
            info.size = boost::filesystem::file_size(file_to_scan, ec);
            info.modified_time = (!ec) ?
                std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(file_to_scan, ec)) :
                std::chrono::system_clock::time_point();
            if(!ec) {
                m_fileIndexList.push_back(info);
            }
        } else {
            m_filesSkippedInIndexing.push_back(file_to_scan);
            GHULBUS_LOG(Warning, "File type " << status.type() << " for " << file_to_scan << " is not supported. "
                                 "File will be skipped.");
        }
    }
    if(ec) {
        m_filesSkippedInIndexing.push_back(file_to_scan);
        GHULBUS_LOG(Warning, "Error while accessing " << file_to_scan << " - " << ec.message() << ". "
                             "File will be skipped.");
    }

    emit indexingUpdate(m_fileIndexList.size());
}

Hash FileScanner::calculateHash(FileInfo const& file_info)
{
    auto fin = std::fopen(file_info.path.string().c_str(), "rb");
    if(!fin) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError(), "Error opening file " + file_info.path.string() + ".");
    }
    auto fin_guard = Ghulbus::finally([&fin]() { int ret = std::fclose(fin); GHULBUS_ASSERT(ret == 0); });
    std::size_t const HASH_BUFFER_SIZE = 4096;
    std::array<char, HASH_BUFFER_SIZE> buffer;
    std::size_t bytes_left = file_info.size;
    std::size_t hash_update = 0;
    CryptoPP::SHA256 hash_calc;
    hash_calc.Restart();
    while(bytes_left > 0) {
        std::size_t const bytes_to_read = std::min(bytes_left, buffer.size());
        auto const bytes_read = std::fread(buffer.data(), sizeof(char), bytes_to_read, fin);
        if(bytes_read != bytes_to_read) {
            GHULBUS_THROW(Ghulbus::Exceptions::IOError(), "Error while reading " + file_info.path.string() + ".");
        }
        bytes_left -= bytes_read;
        hash_update += bytes_read;
        hash_calc.Update(reinterpret_cast<CryptoPP::byte const*>(buffer.data()), bytes_read);
        if (hash_update > (1 << 24)) {
            hash_update = 0;
            emit processingUpdateFileProgress(file_info.size - static_cast<std::uintmax_t>(bytes_left));
        }
    }
    GHULBUS_ASSERT(bytes_left == 0);
    Hash hash;
    hash_calc.Final(reinterpret_cast<CryptoPP::byte*>(hash.digest.data()));
    GHULBUS_LOG(Trace, "Hash for " << file_info.path << " is " << to_string(hash) << ".");
    return hash;
}

class IFileProcessor {
public:
    IFileProcessor(IFileProcessor const&) = delete;
    IFileProcessor& operator=(IFileProcessor const&) = delete;

    virtual ~IFileProcessor() = 0;
    virtual void initialize() = 0;
    virtual void update(std::vector<char> const& chunk) = 0;
    virtual void finalize() = 0;
};

void FileScanner::processFile(FileInfo const& file_info)
{
    auto fin = std::fopen(file_info.path.string().c_str(), "rb");
    if (!fin) {
        GHULBUS_THROW(Ghulbus::Exceptions::IOError(), "Error opening file " + file_info.path.string() + ".");
    }
    auto fin_guard = Ghulbus::finally([&fin]() { int ret = std::fclose(fin); GHULBUS_ASSERT(ret == 0); });
    std::size_t constexpr FILE_BUFFER_SIZE = 4*1024*1024;
    std::vector<char> buffer(FILE_BUFFER_SIZE);

    std::size_t bytes_left = file_info.size;
    std::size_t processing_update = 0;

    IFileProcessor* processor;
    // initialize processor
    processor->initialize();

    while (bytes_left > 0) {
        std::size_t const bytes_to_read = std::min(bytes_left, buffer.size());
        auto const bytes_read = std::fread(buffer.data(), sizeof(char), bytes_to_read, fin);
        if (bytes_read != bytes_to_read) {
            GHULBUS_THROW(Ghulbus::Exceptions::IOError(), "Error while reading " + file_info.path.string() + ".");
        }
        bytes_left -= bytes_read;
        processing_update += bytes_read;

        // update processor
        processor->update(buffer);

        if (processing_update > (1 << 24)) {
            processing_update = 0;
            emit processingUpdateFileProgress(file_info.size - static_cast<std::uintmax_t>(bytes_left));
        }
    }
    GHULBUS_ASSERT(bytes_left == 0);

    // finalize processor
    processor->finalize();
}

void FileScanner::cancelScanning()
{
    m_cancelScanning.store(true);
}

std::unique_ptr<BlimpDB> FileScanner::joinScanning()
{
    m_scanThread.join();
    return std::move(m_dbReturnChannel);
}

void FileScanner::startProcessing(std::vector<FileInfo> const& files, std::unique_ptr<BlimpDB> blimpdb)
{
    GHULBUS_PRECONDITION(!m_scanThread.joinable());
    m_scanThread = std::thread([this, files, blimpdb = std::move(blimpdb)]() {
        std::uintmax_t n = 0;
        std::vector<Hash> hashes;
        hashes.reserve(files.size());
        m_timings.hashingStart = std::chrono::steady_clock::now();
        std::size_t files_processed = 0;
        for (auto const& f : files) {
            emit processingUpdateNewFile(n++, f.size);
            hashes.push_back(calculateHash(f));
            ++files_processed;
            if (m_cancelScanning) {
                GHULBUS_LOG(Debug, "Aborting scanning due to cancel request.");
                return;
            }
        }
        m_timings.hashingFinished = std::chrono::steady_clock::now();
        auto const hashingDuration = m_timings.hashingFinished - m_timings.hashingStart;
        auto const hashingDurationSeconds = std::chrono::duration_cast<std::chrono::seconds>(hashingDuration);
        GHULBUS_LOG(Info, "Hashing took " << hashingDurationSeconds.count() << " seconds.");

        m_timings.indexDbUpdateStart = std::chrono::steady_clock::now();
        auto const file_index_info = blimpdb->updateFileIndex(m_fileIndexList, hashes);
        m_timings.indexDbUpdateFinished = std::chrono::steady_clock::now();
        auto const indexDbUpdateDuration = m_timings.indexDbUpdateFinished - m_timings.indexDbUpdateStart;
        auto const indexDbUpdateDurationMsecs = std::chrono::duration_cast<std::chrono::milliseconds>(indexDbUpdateDuration);
        GHULBUS_LOG(Info, "Database file index updated. Took " << indexDbUpdateDurationMsecs.count() << " milliseconds.");
        emit checksumCalculationCompleted();

        for (auto const& f : files) {
            
            for (std::uintmax_t ss = 0; ss < f.size; ss += 1024 * 1024 * 50) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
            }
        }
        emit processingCompleted();
    });

}

void FileScanner::joinProcessing()
{
    m_scanThread.join();
}

std::vector<FileInfo> const& FileScanner::getIndexList() const
{
    return m_fileIndexList;
}

FileIndexDiff const& FileScanner::getIndexDiff() const
{
    return m_fileDiff;
}
