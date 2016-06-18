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

void FileScanner::addFilesToScan(std::vector<std::string> const& files_to_add)
{
    GHULBUS_LOG(Debug, "Adding " << files_to_add.size() << " item(s) for scanning.");
    std::lock_guard<std::mutex> lk(m_mtx);
    m_filesToScan.insert(begin(m_filesToScan), begin(files_to_add), end(files_to_add));
    m_condvar.notify_one();
}

void FileScanner::startScanning()
{
    m_cancelScanning.store(false);
    m_scanThread = std::thread([this]() {
        std::vector<std::string> files_to_process;
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            files_to_process.reserve(m_filesToScan.size());
            std::move(begin(m_filesToScan), end(m_filesToScan), std::back_inserter(files_to_process));
            m_filesToScan.clear();
        }
        GHULBUS_LOG(Debug, "Indexing " << files_to_process.size() << " item(s) for scanning.");
        for(auto const& f : files_to_process) {
            buildFileListRecursively(f);
            if(m_cancelScanning) {
                GHULBUS_LOG(Debug, "Aborting scanning due to cancel request.");
                return;
            }
        }
        GHULBUS_LOG(Debug, "Indexing complete. Found " << m_fileList.size() << " file(s).");
        emit fileListCompleted(m_fileList.size());
    });
}

void FileScanner::buildFileListRecursively(boost::filesystem::path const& file_to_scan)
{
    if(m_cancelScanning) { return; }
    if(boost::filesystem::is_directory(file_to_scan)) {
        boost::system::error_code ec;
        for(auto const& f : boost::filesystem::directory_iterator(file_to_scan, ec)) {
            if(boost::filesystem::is_directory(f)) {
                buildFileListRecursively(f);
            } else if(boost::filesystem::is_regular_file(f)) {
                FileInfo info;
                info.path = f;
                info.size = boost::filesystem::file_size(f);
                info.modified_time = boost::filesystem::last_write_time(f);
                m_fileList.push_back(info);
            }
        }
        if(ec) {
            GHULBUS_LOG(Warning, "Error while accessing " << file_to_scan << " - " << ec.message() << ". "
                                 "Directory will be skipped.");
        }
    }
}

void FileScanner::cancelScanning()
{
    if(m_scanThread.joinable()) {
        m_cancelScanning.store(true);
        m_scanThread.join();
    }
}
