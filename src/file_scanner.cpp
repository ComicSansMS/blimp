#include <file_scanner.hpp>

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
    std::lock_guard<std::mutex> lk(m_mtx);
    m_filesToScan.insert(begin(m_filesToScan), begin(files_to_add), end(files_to_add));
    m_condvar.notify_one();
}

void FileScanner::startScanning()
{
    m_scanThread = std::thread([this]() {
        std::vector<std::string> files_to_process;
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            files_to_process.reserve(m_filesToScan.size());
            std::move(begin(m_filesToScan), end(m_filesToScan), std::back_inserter(files_to_process));
            m_filesToScan.clear();
        }
        for(auto const& f : files_to_process) {
            buildFileListRecursively(f);
        }
        emit fileListCompleted(m_fileList.size());
    });
}

void FileScanner::buildFileListRecursively(boost::filesystem::path const& file_to_scan)
{
    if(boost::filesystem::is_directory(file_to_scan)) {
        for(auto const& f : boost::filesystem::directory_iterator(file_to_scan)) {
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
    }
}

void FileScanner::cancelScanning()
{
}
