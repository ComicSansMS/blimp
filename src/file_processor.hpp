#ifndef BLIMP_INCLUDE_GUARD_FILE_PROCESSOR_HPP
#define BLIMP_INCLUDE_GUARD_FILE_PROCESSOR_HPP

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
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class ProcessingPipeline;

class FileProcessor : public QObject
{
    Q_OBJECT
private:
    std::atomic<bool> m_cancelProcessing;
    std::mutex m_mtx;
    std::thread m_processingThread;
    std::vector<FileInfo> m_filesToProcess;
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
    std::unique_ptr<ProcessingPipeline> m_processingPipeline;
public:
    FileProcessor();
    ~FileProcessor();
    FileProcessor(FileProcessor const&) = delete;
    FileProcessor& operator=(FileProcessor const&) = delete;

    void startProcessing(BlimpDB::SnapshotId snapshot_id, std::vector<FileInfo>&& files, std::unique_ptr<BlimpDB>&& blimpdb);
    void cancelProcessing();
    [[nodiscard]] std::unique_ptr<BlimpDB> joinProcessing();

signals:
    void processingUpdateNewFile(std::uint64_t current_file_indexed, std::uint64_t current_file_size);
    void processingUpdateHashProgress(std::uint64_t current_file_bytes_processed);
    void processingUpdateHashCompleted(std::uint64_t current_file_indexed, std::uint64_t current_file_size);
    void processingUpdateFileProgress(std::uint64_t current_file_bytes_processed);
    void processingCompleted();
    void processingCanceled();
private:
    Hash calculateHash(FileInfo const& file_info);
};

#endif
