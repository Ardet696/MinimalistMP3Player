#include <chrono>
#include <cstdio>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

#include "../../src/commands/CommandQueue.h"

static constexpr int ITERATIONS = 100;
static constexpr auto CONTENTION_HOLD = std::chrono::milliseconds(50);

int main() {
    // --- Baseline: direct call blocked by mutex contention ---
    {
        std::mutex sharedMutex;
        std::vector<double> times;
        times.reserve(ITERATIONS);

        for (int i = 0; i < ITERATIONS; ++i) {
            // Contender holds the mutex for 50 ms (simulates audio operation)
            std::thread contender([&] {
                std::lock_guard lock(sharedMutex);
                std::this_thread::sleep_for(CONTENTION_HOLD);
            });

            // Give the contender time to acquire
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            auto start = std::chrono::steady_clock::now();
            {
                std::lock_guard lock(sharedMutex); // TUI thread blocks here
                (void)0;
            }
            auto end = std::chrono::steady_clock::now();

            contender.join();
            times.push_back(
                std::chrono::duration<double, std::milli>(end - start).count());
        }

        double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
        double maxT = *std::max_element(times.begin(), times.end());
        std::printf("[Baseline - direct call under contention]\n");
        std::printf("  mean block time : %.2f ms\n", mean);
        std::printf("  max  block time : %.2f ms\n\n", maxT);
    }

    // --- With CommandQueue: enqueue is near-instant ---
    {
        CommandQueue queue;
        std::mutex sharedMutex;
        std::vector<double> times;
        times.reserve(ITERATIONS);

        for (int i = 0; i < ITERATIONS; ++i) {
            std::thread contender([&] {
                std::lock_guard lock(sharedMutex);
                std::this_thread::sleep_for(CONTENTION_HOLD);
            });

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            auto start = std::chrono::steady_clock::now();
            queue.enqueue([&sharedMutex] {
                std::lock_guard lock(sharedMutex); // worker blocks, not TUI
                (void)0;
            });
            auto end = std::chrono::steady_clock::now();

            contender.join();
            times.push_back(
                std::chrono::duration<double, std::micro>(end - start).count());
        }

        double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
        double maxT = *std::max_element(times.begin(), times.end());
        std::printf("[CommandQueue - enqueue cost for TUI thread]\n");
        std::printf("  mean enqueue time : %.2f µs\n", mean);
        std::printf("  max  enqueue time : %.2f µs\n", maxT);
    }

    return 0;
}
