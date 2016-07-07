#include <file_scanner.hpp>

#include <exceptions.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>

#include <sha.h>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <array>
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
    m_cancelScanning.store(false);
    m_scanThread = std::thread([this, blimpdb = std::move(blimpdb)]() {
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

        // todo: check db to calculate index diff with earlier scan
        blimpdb->compareFileIndex(m_fileIndexList);

        std::size_t files_processed = 0;
        std::vector<Hash> hashes;
        hashes.reserve(m_fileIndexList.size());
        m_timings.hashingStart = std::chrono::steady_clock::now();
        for(auto const& f : m_fileIndexList) {
            hashes.push_back(calculateHash(f));
            ++files_processed;
            emit checksumCalculationUpdate(files_processed);
            if(m_cancelScanning) {
                GHULBUS_LOG(Debug, "Aborting scanning due to cancel request.");
                break;
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
    auto fin_guard = gsl::finally([&fin]() { int ret = std::fclose(fin); GHULBUS_ASSERT(ret == 0); });
    std::size_t const HASH_BUFFER_SIZE = 4096;
    std::array<gsl::byte, HASH_BUFFER_SIZE> buffer;
    std::size_t bytes_left = file_info.size;
    CryptoPP::SHA256 hash_calc;
    hash_calc.Restart();
    while(bytes_left > 0) {
        std::size_t const bytes_to_read = std::min(bytes_left, buffer.size());
        auto const bytes_read = std::fread(buffer.data(), sizeof(gsl::byte), bytes_to_read, fin);
        if(bytes_read != bytes_to_read) {
            GHULBUS_THROW(Ghulbus::Exceptions::IOError(), "Error while reading " + file_info.path.string() + ".");
        }
        bytes_left -= bytes_read;
        hash_calc.Update(reinterpret_cast<byte const*>(buffer.data()), bytes_read);
    }
    GHULBUS_ASSERT(bytes_left == 0);
    Hash hash;
    hash_calc.Final(reinterpret_cast<byte*>(hash.digest.data()));
    GHULBUS_LOG(Trace, "Hash for " << file_info.path << " is " << to_string(hash) << ".");
    return hash;
}

void FileScanner::cancelScanning()
{
    if(m_scanThread.joinable()) {
        m_cancelScanning.store(true);
        m_scanThread.join();
    }
}
