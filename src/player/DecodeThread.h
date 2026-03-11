#ifndef MP3PLAYER_DECODETHREAD_H
#define MP3PLAYER_DECODETHREAD_H

#include <atomic>
#include <thread>
#include <cstdint>
#include "../decode/Mp3Decoder.h"
#include "../util/RingBuffer.h"

/**
 * Manages a background thread that continuously decodes MP3 data into a ring buffer.
 *
 * The decode thread runs independently, filling the ring buffer with decoded PCM samples.
 * The audio callback reads from the ring buffer without blocking the decoder.
 */
class DecodeThread {
public:
    DecodeThread();
    ~DecodeThread();

    // Non-copyable, non-movable (owns thread)
    DecodeThread(const DecodeThread&) = delete;
    DecodeThread& operator=(const DecodeThread&) = delete;
    DecodeThread(DecodeThread&&) = delete;
    DecodeThread& operator=(DecodeThread&&) = delete;

    /**
     * Start the decode thread.
     *
     * @param decoder - The MP3 decoder to read from (must outlive this object)
     * @param ringBuffer - The ring buffer to write to (must outlive this object)
     * @param chunkFrames - Number of frames to decode per iteration
     */
    void start(Mp3Decoder* decoder, RingBuffer<int16_t>* ringBuffer, std::size_t chunkFrames = 1152);
    void stop();
    bool isEndOfStream() const;
    bool isRunning() const;

private:
    /**
     * The main decode loop that runs on the background thread.
     */
    void decodeLoop(std::stop_token stopToken);

    Mp3Decoder* decoder_;              // Not owned
    RingBuffer<int16_t>* ringBuffer_;  // Not owned
    std::size_t chunkFrames_;

    std::jthread thread_;
    std::atomic<bool> endOfStream_;
    std::atomic<bool> running_;
};

#endif
