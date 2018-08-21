#include <file_processor.hpp>

#include <file_bundling.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

struct FileProcessor::Pimpl
{
    std::vector<FileInfo> file_list;
    std::thread worker_thread;
    std::mutex mutex;
    bool stop_requested;
    std::deque<ProcessedElement> processed_elements;
    std::condition_variable elements_available;
};

FileProcessor::FileProcessor(std::vector<FileInfo> const& files)
    :m_pimpl(std::make_unique<Pimpl>())
{
    m_pimpl->file_list = files;
}

FileProcessor::~FileProcessor()
{
}

void FileProcessor::startProcessing()
{
    GHULBUS_PRECONDITION(!m_pimpl->worker_thread.joinable());
    m_pimpl->stop_requested = false;
    m_pimpl->worker_thread = std::thread([this]() {
        process();
    });
}

void FileProcessor::stopProcessing()
{
    GHULBUS_PRECONDITION(m_pimpl->worker_thread.joinable());
    m_pimpl->worker_thread.join();
}

void FileProcessor::process()
{
    std::unique_lock<std::mutex> lk(m_pimpl->mutex);
    std::uintmax_t const MIN_BUNDLE_SIZE = 1024*1024*1024; // 1MB
    auto const& file_list = m_pimpl->file_list;
    auto const bundle = bundleFiles(file_list, MIN_BUNDLE_SIZE);
    for(int i = 0; i < file_list.size(); ++i) {
        if(bundle[i].bundle_id == 0) {
            // standalone file
            
        }
    }
}

ProcessedElement FileProcessor::getProcessedElement()
{
    std::unique_lock<std::mutex> lk(m_pimpl->mutex);
    GHULBUS_PRECONDITION(!m_pimpl->stop_requested);
    m_pimpl->elements_available.wait(lk, [this]() { return !m_pimpl->processed_elements.empty(); });
    ProcessedElement ret = m_pimpl->processed_elements.front();
    m_pimpl->processed_elements.pop_front();
    return ret;
}

std::size_t FileProcessor::getRemainingElements() const
{
    std::lock_guard<std::mutex> lk(m_pimpl->mutex);
    return m_pimpl->file_list.size();
}
