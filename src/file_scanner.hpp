#ifndef BLIMP_INCLUDE_GUARD_FILE_SCANNER_HPP
#define BLIMP_INCLUDE_GUARD_FILE_SCANNER_HPP

#include <QObject>

#include <db/blimpdb.hpp>
#include <file_hash.hpp>
#include <file_info.hpp>

#include <boost/filesystem/path.hpp>

#include <atomic>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

class FileScanner : public QObject
{
    Q_OBJECT
private:
    std::atomic<bool> m_cancelScanning;
    std::deque<std::string> m_filesToIndex;
    std::mutex m_mtx;
    std::thread m_scanThread;
    std::vector<FileInfo> m_fileIndexList;
    FileIndexDiff m_fileDiff;
    std::vector<boost::filesystem::path> m_filesSkippedInIndexing;
    struct Timings {
        std::chrono::steady_clock::time_point indexingStart;
        std::chrono::steady_clock::time_point indexingFinished;
        std::chrono::steady_clock::time_point indexDiffComputationStart;
        std::chrono::steady_clock::time_point indexDiffComputationFinished;
        std::chrono::steady_clock::time_point hashingStart;
        std::chrono::steady_clock::time_point hashingFinished;
        std::chrono::steady_clock::time_point indexDbUpdateStart;
        std::chrono::steady_clock::time_point indexDbUpdateFinished;
    } m_timings;
    std::unique_ptr<BlimpDB> m_dbReturnChannel;
public:
    FileScanner();
    ~FileScanner();
    FileScanner(FileScanner const&) = delete;
    FileScanner& operator=(FileScanner const&) = delete;

    void addFilesForIndexing(std::vector<std::string> const& files_to_add);

    void startScanning(std::unique_ptr<BlimpDB> blimpdb);
    void cancelScanning();
    [[nodiscard]] std::unique_ptr<BlimpDB> joinScanning();

    void startProcessing(std::vector<FileInfo> const& files, std::unique_ptr<BlimpDB> blimpdb);
    void joinProcessing();

    std::vector<FileInfo> const& getIndexList() const;
    FileIndexDiff const& getIndexDiff() const;

signals:
    void indexingCompleted(std::uint64_t n_files_indexed);
    void indexingUpdate(std::uint64_t n_files_indexed);

    void indexDiffCompleted();

    void processingUpdateNewFile(std::uint64_t current_file_indexed, std::uint64_t current_file_size);
    void processingUpdateFileProgress(std::uint64_t current_file_bytes_processed);
    void processingCompleted();

    void checksumCalculationUpdate(std::uint64_t n_total_files_processed);
    void checksumCalculationCompleted();

private:
    void indexFilesRecursively(boost::filesystem::path const& file_to_scan);

    Hash calculateHash(FileInfo const& file_info);
};

#endif
