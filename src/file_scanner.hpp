#ifndef BLIMP_INCLUDE_GUARD_FILE_SCANNER_HPP
#define BLIMP_INCLUDE_GUARD_FILE_SCANNER_HPP

#include <QObject>

#include <boost/filesystem.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

class FileScanner : public QObject
{
    Q_OBJECT
public:
    struct FileInfo {
        boost::filesystem::path path;
        std::uintmax_t size;
        std::chrono::system_clock::time_point modified_time;
    };
private:
    std::deque<std::string> m_filesToIndex;
    std::mutex m_mtx;
    std::condition_variable m_condvar;
    std::thread m_scanThread;
    std::vector<FileInfo> m_fileIndexList;
    std::vector<boost::filesystem::path> m_filesSkippedInIndexing;
    std::atomic<bool> m_cancelScanning;
    struct Timings {
        std::chrono::steady_clock::time_point indexingStart;
        std::chrono::steady_clock::time_point indexingFinished;
    } m_timings;
public:
    FileScanner();
    ~FileScanner();
    FileScanner(FileScanner const&) = delete;
    FileScanner& operator=(FileScanner const&) = delete;

    void addFilesForIndexing(std::vector<std::string> const& files_to_add);

public slots:
    void startScanning();
    void cancelScanning();

signals:
    void fileListCompleted(std::uintmax_t n_files_in_list);
    void indexUpdate(std::uintmax_t n_files_indexed);

private:
    void indexFilesRecursively(boost::filesystem::path const& file_to_scan);
};

#endif
