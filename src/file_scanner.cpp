#include <file_scanner.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <gsl.h>

#include <hex.h>
#include <sha.h>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <array>
#include <cstdio>

std::string digestToString(std::array<gsl::byte, CryptoPP::SHA256::DIGESTSIZE> const& digest)
{
    CryptoPP::HexEncoder enc;
    enc.Put(reinterpret_cast<byte const*>(digest.data()), digest.size());
    std::array<char, 2*CryptoPP::SHA256::DIGESTSIZE + 1> buffer;
    auto const res = enc.Get(reinterpret_cast<byte*>(buffer.data()), 2*digest.size());
    GHULBUS_ASSERT(res == 2*digest.size());
    buffer.back() = '\0';
    return std::string(buffer.data());
}

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
    m_condvar.notify_one();
}

void FileScanner::startScanning()
{
    m_cancelScanning.store(false);
    m_scanThread = std::thread([this]() {
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
        GHULBUS_LOG(Debug, "Indexing complete after " << indexingDurationSeconds.count() << " seconds."
                           " Found " << m_fileIndexList.size() << " file(s).");
        emit indexingCompleted(m_fileIndexList.size());

        std::size_t files_processed = 0;
        for(auto const& f : m_fileIndexList) {
            calculateHash(f);
            ++files_processed;
            emit checksumCalculationUpdate(files_processed);
            if(m_cancelScanning) {
                GHULBUS_LOG(Debug, "Aborting scanning due to cancel request.");
                break;
            }
        }
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

void FileScanner::calculateHash(FileInfo const& file_info)
{
    auto fin = std::fopen(file_info.path.string().c_str(), "rb");
    if(!fin) {
    }
    auto fin_guard = gsl::finally([&fin]() { int ret = std::fclose(fin); GHULBUS_ASSERT(ret == 0); });
    std::size_t const HASH_BUFFER_SIZE = 8192;
    std::array<gsl::byte, HASH_BUFFER_SIZE> buffer;
    std::size_t bytes_left = file_info.size;
    CryptoPP::SHA256 hash_calc;
    hash_calc.Restart();
    while(bytes_left > 0) {
        std::size_t const bytes_to_read = std::min(bytes_left, buffer.size());
        auto const bytes_read = std::fread(buffer.data(), sizeof(gsl::byte), bytes_to_read, fin);
        if(bytes_read != bytes_to_read) {
            GHULBUS_LOG(Error, "Error while reading file " << file_info.path << "; " << bytes_read << ".");
            break;
        }
        bytes_left -= bytes_read;
        hash_calc.Update(reinterpret_cast<byte const*>(buffer.data()), bytes_read);
    }
    if(bytes_left == 0) {
        std::array<gsl::byte, CryptoPP::SHA256::DIGESTSIZE> hash;
        hash_calc.Final(reinterpret_cast<byte*>(hash.data()));
        GHULBUS_LOG(Debug, "Hash for " << file_info.path << " is " << digestToString(hash) << ".");
    }
}

void FileScanner::cancelScanning()
{
    if(m_scanThread.joinable()) {
        m_cancelScanning.store(true);
        m_scanThread.join();
    }
}
