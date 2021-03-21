#include <worker_pool.hpp>

#include <gbBase/Log.hpp>

WorkerPool::WorkerPool(std::size_t n_threads)
    :m_done(false)
{
    for (std::size_t i = 0; i < n_threads; ++i) {
        m_threads.emplace_back([this]() { work(); });
    }
}

WorkerPool::~WorkerPool()
{
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_done = true;
    }
    m_cv.notify_all();
    for (auto& t : m_threads) { t.join(); }
}

void WorkerPool::schedule(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_tasks.emplace_back(std::move(task));
    }
    m_cv.notify_one();
}

void WorkerPool::work()
{
    std::unique_lock<std::mutex> lk(m_mtx);
    for (;;) {
        m_cv.wait(lk, [this]() { return (!m_tasks.empty()) || m_done; });
        if (m_tasks.empty()) { break; }
        auto const t = std::move(m_tasks.front());
        m_tasks.pop_front();
        lk.unlock();
        try {
            t();
        } catch (...) {
            GHULBUS_LOG(Warning, "Exception encountered in worker pool.");
        }
        lk.lock();
    }
}
