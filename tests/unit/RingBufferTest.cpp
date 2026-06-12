#include <catch2/catch_all.hpp>
#include "../../src/util/RingBuffer.h"
#include <vector>
#include <numeric>

TEST_CASE("RingBuffer capacity rounding", "[ringbuffer]") {
    CHECK(RingBuffer<int>(0).capacity() == 1);
    CHECK(RingBuffer<int>(1).capacity() == 1);
    CHECK(RingBuffer<int>(2).capacity() == 2);
    CHECK(RingBuffer<int>(3).capacity() == 4);
    CHECK(RingBuffer<int>(5).capacity() == 8);
    CHECK(RingBuffer<int>(1024).capacity() == 1024);
    CHECK(RingBuffer<int>(1025).capacity() == 2048);
}

TEST_CASE("RingBuffer empty state", "[ringbuffer]") {
    RingBuffer<int16_t> rb(64);
    CHECK(rb.isEmpty());
    CHECK(rb.availableToRead() == 0);
    CHECK(rb.availableToWrite() == 64);
}

TEST_CASE("RingBuffer write and read", "[ringbuffer]") {
    RingBuffer<int16_t> rb(16);
    int16_t writeData[] = {1, 2, 3, 4, 5};
    int16_t readData[5] = {};

    SECTION("basic write then read") {
        CHECK(rb.write(writeData, 5) == 5);
        CHECK(rb.availableToRead() == 5);
        CHECK(rb.availableToWrite() == 11);
        CHECK(rb.read(readData, 5) == 5);
        for (int i = 0; i < 5; i++)
            CHECK(readData[i] == writeData[i]);
        CHECK(rb.isEmpty());
    }

    SECTION("read from empty returns 0") {
        CHECK(rb.read(readData, 5) == 0);
    }

    SECTION("partial read") {
        rb.write(writeData, 5);
        CHECK(rb.read(readData, 3) == 3);
        CHECK(rb.availableToRead() == 2);
        CHECK(readData[0] == 1);
        CHECK(readData[1] == 2);
        CHECK(readData[2] == 3);
    }
}

TEST_CASE("RingBuffer fill to capacity", "[ringbuffer]") {
    RingBuffer<int> rb(8);
    std::vector<int> data(8);
    std::iota(data.begin(), data.end(), 100);

    CHECK(rb.write(data.data(), 8) == 8);
    CHECK(rb.availableToWrite() == 0);

    SECTION("write when full returns 0") {
        int extra = 999;
        CHECK(rb.write(&extra, 1) == 0);
    }

    SECTION("partial write when nearly full") {
        RingBuffer<int> rb2(8);
        std::vector<int> first(6, 1);
        rb2.write(first.data(), 6);
        std::vector<int> second(4, 2);
        CHECK(rb2.write(second.data(), 4) == 2);
    }

    SECTION("read back full buffer") {
        std::vector<int> out(8);
        CHECK(rb.read(out.data(), 8) == 8);
        for (int i = 0; i < 8; i++)
            CHECK(out[i] == 100 + i);
    }
}

TEST_CASE("RingBuffer wraparound correctness", "[ringbuffer]") {
    RingBuffer<int> rb(8);

    // Fill halfway and drain — advances head and tail past the start
    std::vector<int> fill(6, 0);
    rb.write(fill.data(), 6);
    rb.read(fill.data(), 6);

    // Now write data that wraps around the buffer boundary
    std::vector<int> wrapData = {10, 20, 30, 40, 50, 60, 70, 80};
    size_t written = rb.write(wrapData.data(), 8);
    CHECK(written == 8);

    std::vector<int> out(8);
    size_t readCount = rb.read(out.data(), 8);
    CHECK(readCount == 8);

    for (size_t i = 0; i < 8; i++)
        CHECK(out[i] == wrapData[i]);
}

TEST_CASE("RingBuffer clear resets state", "[ringbuffer]") {
    RingBuffer<int16_t> rb(32);
    int16_t data[] = {1, 2, 3};
    rb.write(data, 3);
    CHECK(!rb.isEmpty());

    rb.clear();
    CHECK(rb.isEmpty());
    CHECK(rb.availableToRead() == 0);
    CHECK(rb.availableToWrite() == 32);
}

TEST_CASE("RingBuffer sequential fill-drain cycles", "[ringbuffer]") {
    RingBuffer<int> rb(16);

    // Run multiple fill/drain cycles to stress wraparound arithmetic
    for (int cycle = 0; cycle < 100; cycle++) {
        std::vector<int> data(16);
        std::iota(data.begin(), data.end(), cycle * 16);
        CHECK(rb.write(data.data(), 16) == 16);

        std::vector<int> out(16);
        CHECK(rb.read(out.data(), 16) == 16);
        for (int i = 0; i < 16; i++)
            CHECK(out[i] == cycle * 16 + i);
    }
}
