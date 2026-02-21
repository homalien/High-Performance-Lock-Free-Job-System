# cpp-lockfree-job-system

High-performance lock-free task scheduler written in modern C++20.
Designed for low-latency, CPU-bound parallel workloads.

---

## Overview

cpp-lockfree-job-system is a work-stealing task runtime built around a
Chase–Lev deque design. It minimizes synchronization overhead by eliminating
global locks and leveraging atomic memory ordering semantics.

Goals:

- High task throughput
- Low dispatch latency
- Linear scalability across cores
- Mechanical sympathy with modern CPU architectures
- Deterministic shutdown behavior

This project focuses on correctness, performance, and predictable scheduling.

---

## Architecture

| Layer        | Component           | Responsibility                      | Techniques Used             |
|-------------|--------------------|--------------------------------------|-----------------------------|
| API         | JobSystem          | Task submission & lifecycle          | Thread pool                 |
| Scheduling  | WorkStealingQueue  | Per-thread double-ended deque        | Lock-free ring buffer       |
| Execution   | Worker Threads     | Task execution loop                  | Work stealing               |
| Memory      | Internal buffers   | Cache-line aligned storage           | False-sharing mitigation    |

---

## Concurrency Model

- Per-thread work-stealing deque
- Owner thread performs push/pop (LIFO)
- Thief threads perform steal (FIFO)
- Atomic operations with explicit memory ordering
- No global locks in hot paths

---

## Performance Characteristics

- O(1) task push/pop in uncontended path
- Lock-free scheduling
- Reduced cache contention
- Near-linear scalability under CPU-bound load

---

## Example Usage

```cpp
#include "job_system.hpp"
#include <iostream>

int main() {
    JobSystem scheduler(8);

    for (int i = 0; i < 100000; ++i) {
        scheduler.submit([i]() {
            volatile int x = i * i;
        });
    }

    scheduler.shutdown();
}
