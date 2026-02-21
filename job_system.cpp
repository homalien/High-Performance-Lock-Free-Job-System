#include "job_system.hpp"

JobSystem::JobSystem(size_t thread_count)
    : running_(true) {

    queues_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i)
        queues_.emplace_back(std::make_unique<WorkStealingQueue>());

    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this, i]() {
            worker_loop(i);
        });
    }
}

JobSystem::~JobSystem() {
    shutdown();
}

void JobSystem::submit(WorkStealingQueue::Job job) {
    size_t index = std::hash<std::thread::id>{}(std::this_thread::get_id()) % queues_.size();
    queues_[index]->push(std::move(job));
}

void JobSystem::worker_loop(size_t index) {
    while (running_) {
        WorkStealingQueue::Job job;

        if (queues_[index]->pop(job)) {
            job();
        } else {
            for (size_t i = 0; i < queues_.size(); ++i) {
                if (i != index && queues_[i]->steal(job)) {
                    job();
                    break;
                }
            }
        }
    }
}

void JobSystem::shutdown() {
    running_ = false;
    for (auto& t : workers_)
        if (t.joinable())
            t.join();
}
