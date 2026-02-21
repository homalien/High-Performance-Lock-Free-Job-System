#pragma once

#include "work_stealing_queue.hpp"
#include <thread>
#include <vector>
#include <atomic>

class JobSystem {
public:
    explicit JobSystem(size_t thread_count = std::thread::hardware_concurrency());
    ~JobSystem();

    void submit(WorkStealingQueue::Job job);
    void shutdown();

private:
    void worker_loop(size_t index);

    std::vector<std::thread> workers_;
    std::vector<std::unique_ptr<WorkStealingQueue>> queues_;
    std::atomic<bool> running_;
};
