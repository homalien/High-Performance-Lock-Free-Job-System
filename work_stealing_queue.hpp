#pragma once

#include <atomic>
#include <vector>
#include <cstddef>
#include <functional>

class WorkStealingQueue {
public:
    using Job = std::function<void()>;

    explicit WorkStealingQueue(size_t capacity = 1024)
        : buffer_(capacity),
          top_(0),
          bottom_(0),
          capacity_(capacity) {}

    bool push(Job job) {
        size_t bottom = bottom_.load(std::memory_order_relaxed);
        size_t top = top_.load(std::memory_order_acquire);

        if (bottom - top >= capacity_)
            return false; // full

        buffer_[bottom % capacity_] = std::move(job);
        bottom_.store(bottom + 1, std::memory_order_release);
        return true;
    }

    bool pop(Job& job) {
        size_t bottom = bottom_.load(std::memory_order_relaxed) - 1;
        bottom_.store(bottom, std::memory_order_relaxed);

        size_t top = top_.load(std::memory_order_acquire);

        if (top <= bottom) {
            job = std::move(buffer_[bottom % capacity_]);
            if (top == bottom) {
                if (!top_.compare_exchange_strong(
                        top, top + 1,
                        std::memory_order_seq_cst,
                        std::memory_order_relaxed)) {
                    bottom_.store(bottom + 1, std::memory_order_relaxed);
                    return false;
                }
                bottom_.store(bottom + 1, std::memory_order_relaxed);
            }
            return true;
        } else {
            bottom_.store(bottom + 1, std::memory_order_relaxed);
            return false;
        }
    }

    bool steal(Job& job) {
        size_t top = top_.load(std::memory_order_acquire);
        size_t bottom = bottom_.load(std::memory_order_acquire);

        if (top >= bottom)
            return false;

        job = std::move(buffer_[top % capacity_]);

        return top_.compare_exchange_strong(
            top, top + 1,
            std::memory_order_seq_cst,
            std::memory_order_relaxed);
    }

private:
    std::vector<Job> buffer_;
    std::atomic<size_t> top_;
    std::atomic<size_t> bottom_;
    size_t capacity_;
};
