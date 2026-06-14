#include <chrono>
#include <cstdio>
#include <cstring>
#include <numeric>
#include <thread>
#include <vector>

#include "../../src/util/RingBuffer.h"

static constexpr int    SAMPLE_RATE   = 44100;
static constexpr int    CHANNELS      = 2;
static constexpr size_t CHUNK_FRAMES  = 1152;
static constexpr size_t CHUNK_SAMPLES = CHUNK_FRAMES * CHANNELS;
static constexpr size_t BUF_CAPACITY  = 2 * SAMPLE_RATE * CHANNELS; // 2 s
static constexpr int    ITERATIONS    = 5000;

int main() {
    // --- Single-thread write latency ---
    {
        RingBuffer<int16_t> rb(BUF_CAPACITY);
        std::vector<int16_t> src(CHUNK_SAMPLES, 0);
        std::vector<double> times;
        times.reserve(ITERATIONS);

        for (int i = 0; i < ITERATIONS; ++i) {
            // drain first so there's always room
            std::vector<int16_t> tmp(CHUNK_SAMPLES);
            rb.read(tmp.data(), CHUNK_SAMPLES);

            auto t0 = std::chrono::steady_clock::now();
            rb.write(src.data(), CHUNK_SAMPLES);
            auto t1 = std::chrono::steady_clock::now();

            times.push_back(
                std::chrono::duration<double, std::nano>(t1 - t0).count());
        }

        double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
        double maxT = *std::max_element(times.begin(), times.end());
        std::printf("[RingBuffer write — %zu samples/chunk]\n", CHUNK_SAMPLES);
        std::printf("  mean : %.1f ns\n", mean);
        std::printf("  max  : %.1f ns\n\n", maxT);
    }

    // --- Sustained producer/consumer throughput ---
    {
        RingBuffer<int16_t> rb(BUF_CAPACITY);
        std::vector<int16_t> src(CHUNK_SAMPLES, 0);
        std::vector<int16_t> dst(CHUNK_SAMPLES);

        std::atomic<bool> done{false};
        std::atomic<uint64_t> samplesConsumed{0};

        std::thread consumer([&] {
            while (!done.load(std::memory_order_relaxed)) {
                size_t n = rb.read(dst.data(), CHUNK_SAMPLES);
                samplesConsumed.fetch_add(n, std::memory_order_relaxed);
            }
            // drain remainder
            size_t n;
            while ((n = rb.read(dst.data(), CHUNK_SAMPLES)) > 0)
                samplesConsumed.fetch_add(n, std::memory_order_relaxed);
        });

        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            while (rb.write(src.data(), CHUNK_SAMPLES) == 0)
                std::this_thread::yield();
        }
        done.store(true, std::memory_order_release);
        consumer.join();
        auto end = std::chrono::steady_clock::now();

        double elapsedSec = std::chrono::duration<double>(end - start).count();
        double samplesPerSec = static_cast<double>(samplesConsumed.load()) / elapsedSec;
        double realTimeMultiple = samplesPerSec / static_cast<double>(SAMPLE_RATE * CHANNELS);

        std::printf("[RingBuffer sustained throughput — producer+consumer]\n");
        std::printf("  %.0f samples/sec  (%.0fx real-time at %d Hz stereo)\n\n",
            samplesPerSec, realTimeMultiple, SAMPLE_RATE);
    }

    return 0;
}
