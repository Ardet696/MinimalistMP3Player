#ifndef MP3PLAYER_RINGBUFFER_H
#define MP3PLAYER_RINGBUFFER_H

#include <atomic>
#include <cstddef>
#include <cstring>
#include <memory>
#include <algorithm>

/**
 * Lock free single producer single consumer (SPSC) ring buffer for audio frames.
 */
template<typename T>
class RingBuffer {
public:
    /**
     * Create a ring buffer with the specified capacity. Actual capacity will be rounded up to the next power of 2 for efficiency.
     */
    explicit RingBuffer(std::size_t capacity)
        : capacity_(nextPowerOfTwo(capacity))
        , mask_(capacity_ - 1)
        , buffer_(std::make_unique<T[]>(capacity_))
        , writePos_(0)
        , readPos_(0)
    {
    }

    // Non-copyable, non-movable (owns buffer and has atomic members)
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    /**
     * Write data into the ring buffer.
     * Returns the number of elements actually written (may be less than requested if full).
     * This should be called from the PRODUCER thread only.
     */
    std::size_t write(const T* data, std::size_t count) {
        // Get current positions
        const std::size_t currentWrite = writePos_.load(std::memory_order_relaxed);
        const std::size_t currentRead = readPos_.load(std::memory_order_acquire);

        // Calculate available space
        const std::size_t available = capacity_ - (currentWrite - currentRead);
        const std::size_t toWrite = std::min(count, available);

        if (toWrite == 0) {
            return 0; // Buffer is full
        }

        // Write in up to two parts (handle wraparound)
        const std::size_t writeIndex = currentWrite & mask_;
        const std::size_t firstPart = std::min(toWrite, capacity_ - writeIndex);
        const std::size_t secondPart = toWrite - firstPart;

        // Copy first part
        std::memcpy(buffer_.get() + writeIndex, data, firstPart * sizeof(T));

        // Copy second part if needed (wraparound)
        if (secondPart > 0) {
            std::memcpy(buffer_.get(), data + firstPart, secondPart * sizeof(T));
        }

        // Update write position (release semantics ensures data is visible to reader)
        writePos_.store(currentWrite + toWrite, std::memory_order_release);

        return toWrite;
    }

    /**
     * Read data from the ring buffer.
     * Returns the number of elements actually read (may be less than requested if empty).
     * This should be called from the CONSUMER thread only.
     */
    std::size_t read(T* data, std::size_t count) {
        // Get current positions
        const std::size_t currentRead = readPos_.load(std::memory_order_relaxed);
        const std::size_t currentWrite = writePos_.load(std::memory_order_acquire);

        // Calculate available data
        const std::size_t available = currentWrite - currentRead;
        const std::size_t toRead = std::min(count, available);

        if (toRead == 0) {
            return 0; // Buffer is empty
        }

        // Read in up to two parts (handle wraparound)
        const std::size_t readIndex = currentRead & mask_;
        const std::size_t firstPart = std::min(toRead, capacity_ - readIndex);
        const std::size_t secondPart = toRead - firstPart;

        // Copy first part
        std::memcpy(data, buffer_.get() + readIndex, firstPart * sizeof(T));

        // Copy second part if needed (wraparound)
        if (secondPart > 0) {
            std::memcpy(data + firstPart, buffer_.get(), secondPart * sizeof(T));
        }

        // Update read position (release semantics for symmetry, though not strictly needed)
        readPos_.store(currentRead + toRead, std::memory_order_release);

        return toRead;
    }

    /**
     * Get the number of elements available to read.
     */
    std::size_t availableToRead() const {
        const std::size_t currentRead = readPos_.load(std::memory_order_relaxed);
        const std::size_t currentWrite = writePos_.load(std::memory_order_acquire);
        return currentWrite - currentRead;
    }

    /**
     * Get the number of elements that can be written.
     */
    std::size_t availableToWrite() const {
        const std::size_t currentWrite = writePos_.load(std::memory_order_relaxed);
        const std::size_t currentRead = readPos_.load(std::memory_order_acquire);
        return capacity_ - (currentWrite - currentRead);
    }

    /**
     * Check if the buffer is empty.
     */
    bool isEmpty() const {
        return availableToRead() == 0;
    }

    /**
     * Get the total capacity of the buffer.
     */
    std::size_t capacity() const {
        return capacity_;
    }

    /**
     * Clear all data from the buffer.
     * WARNING: This is NOT thread-safe and should only be called when no threads are accessing the buffer.
     */
    void clear() {
        writePos_.store(0, std::memory_order_relaxed);
        readPos_.store(0, std::memory_order_relaxed);
    }

private:
    /**
     * Round up to the next power of 2.
     * This allows us to use bitwise AND for modulo operations (faster).
     */
    static std::size_t nextPowerOfTwo(std::size_t n) {
        if (n == 0) return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }

    const std::size_t capacity_;  // Power of 2
    const std::size_t mask_;      // capacity_ - 1, for fast modulo
    std::unique_ptr<T[]> buffer_; // The actual data storage

    // Atomic positions for lock-free synchronization
    // These use relaxed/acquire/release memory ordering for performance
    std::atomic<std::size_t> writePos_;
    std::atomic<std::size_t> readPos_;
};

#endif // MP3PLAYER_RINGBUFFER_H
