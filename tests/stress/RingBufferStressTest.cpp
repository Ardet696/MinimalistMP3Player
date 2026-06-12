#include <catch2/catch_all.hpp>
#include "../../src/util/RingBuffer.h"
#include <thread>
#include <atomic>
#include <vector>

TEST_CASE("RingBuffer SPSC concurrent correctness", "[ringbuffer][stress]") {
    constexpr size_t BUFFER_SIZE = 4096;
    constexpr size_t TOTAL_SAMPLES = 10'000'000;
    RingBuffer<int32_t> rb(BUFFER_SIZE);

    std::atomic<bool> producerDone{false};
    std::atomic<size_t> totalWritten{0};
    std::atomic<size_t> totalRead{0};

    // Producer: writes incrementing integers
    std::thread producer([&] {
        size_t written = 0;
        std::vector<int32_t> chunk(512);
        while (written < TOTAL_SAMPLES) {
            size_t chunkSize = std::min<size_t>(512, TOTAL_SAMPLES - written);
            for (size_t i = 0; i < chunkSize; i++)
                chunk[i] = static_cast<int32_t>(written + i);

            size_t n = rb.write(chunk.data(), chunkSize);
            written += n;
            if (n == 0)
                std::this_thread::yield();
        }
        totalWritten.store(written, std::memory_order_release);
        producerDone.store(true, std::memory_order_release);
    });

    // Consumer: reads and verifies ordering
    std::thread consumer([&] {
        size_t readCount = 0;
        std::vector<int32_t> chunk(512);
        while (true) {
            size_t n = rb.read(chunk.data(), 512);
            for (size_t i = 0; i < n; i++) {
                REQUIRE(chunk[i] == static_cast<int32_t>(readCount + i));
            }
            readCount += n;
            if (n == 0) {
                if (producerDone.load(std::memory_order_acquire) && rb.isEmpty())
                    break;
                std::this_thread::yield();
            }
        }
        totalRead.store(readCount, std::memory_order_release);
    });

    producer.join();
    consumer.join();

    CHECK(totalWritten.load() == TOTAL_SAMPLES);
    CHECK(totalRead.load() == TOTAL_SAMPLES);
}

TEST_CASE("RingBuffer SPSC variable chunk sizes", "[ringbuffer][stress]") {
    constexpr size_t BUFFER_SIZE = 1024;
    constexpr size_t TOTAL_SAMPLES = 2'000'000;
    RingBuffer<int16_t> rb(BUFFER_SIZE);

    std::atomic<bool> producerDone{false};
    size_t consumerTotal = 0;

    // Producer: variable-sized writes (1 to 1024)
    std::thread producer([&] {
        size_t written = 0;
        std::vector<int16_t> chunk(1024);
        int chunkVariation = 1;
        while (written < TOTAL_SAMPLES) {
            size_t chunkSize = std::min<size_t>(chunkVariation, TOTAL_SAMPLES - written);
            for (size_t i = 0; i < chunkSize; i++)
                chunk[i] = static_cast<int16_t>((written + i) & 0x7FFF);

            size_t n = rb.write(chunk.data(), chunkSize);
            written += n;
            chunkVariation = (chunkVariation % 1024) + 1;
            if (n == 0)
                std::this_thread::yield();
        }
        producerDone.store(true, std::memory_order_release);
    });

    // Consumer: variable-sized reads
    std::thread consumer([&] {
        std::vector<int16_t> chunk(1024);
        int readVariation = 7;
        while (true) {
            size_t n = rb.read(chunk.data(), readVariation);
            for (size_t i = 0; i < n; i++) {
                int16_t expected = static_cast<int16_t>((consumerTotal + i) & 0x7FFF);
                REQUIRE(chunk[i] == expected);
            }
            consumerTotal += n;
            readVariation = (readVariation % 1024) + 1;
            if (n == 0) {
                if (producerDone.load(std::memory_order_acquire) && rb.isEmpty())
                    break;
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    CHECK(consumerTotal == TOTAL_SAMPLES);
}
