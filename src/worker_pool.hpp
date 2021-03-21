#ifndef BLIMP_INCLUDE_GUARD_WORKER_POOL_HPP
#define BLIMP_INCLUDE_GUARD_WORKER_POOL_HPP

#include <deque>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class WorkerPool {
private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::deque<std::function<void()>> m_tasks;
    bool m_done;
    std::vector<std::thread> m_threads;
public:
    WorkerPool(std::size_t n_threads);
    ~WorkerPool();

    WorkerPool(WorkerPool const&) = delete;
    WorkerPool& operator=(WorkerPool const&) = delete;

    void schedule(std::function<void()> task);
private:
    void work();
};

#endif
