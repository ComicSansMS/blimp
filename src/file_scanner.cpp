#include <file_scanner.hpp>

#include <gbBase/Log.hpp>

#include <boost/filesystem.hpp>

#include <algorithm>

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
        emit fileListCompleted(m_fileIndexList.size());
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

    emit indexUpdate(m_fileIndexList.size());
}

void FileScanner::cancelScanning()
{
    if(m_scanThread.joinable()) {
        m_cancelScanning.store(true);
        m_scanThread.join();
    }
}
