#include "DecodeThread.h"
#include "../config/Config.h"
#include <utility>
#include <vector>
#include <thread>
#include <span>

DecodeThread::DecodeThread()
    : decoder_(nullptr)
    , ringBuffer_(nullptr)
    , chunkFrames_(0)
    , endOfStream_(false)
    , running_(false)
{
}

DecodeThread::~DecodeThread() {
    // jthread automatically requests stop and joins
    stop();
}

void DecodeThread::start(Mp3Decoder* decoder, RingBuffer<int16_t>* ringBuffer, std::size_t chunkFrames) {
    if (running_.load(std::memory_order_acquire)) {
        stop(); // Stop existing thread first
    }

    decoder_ = decoder;
    ringBuffer_ = ringBuffer;
    chunkFrames_ = chunkFrames;
    endOfStream_.store(false, std::memory_order_release);

    // Spawn the decode thread with jthread, use lambda to properly bind member function with stop_token
    thread_ = std::jthread([this](std::stop_token stopToken) {
        decodeLoop(std::move(stopToken)); // avoid unnecessary copies with move
    });
    running_.store(true, std::memory_order_release);
}

void DecodeThread::stop() {
    if (!running_.load(std::memory_order_acquire)) {
        return; // Not running
    }

    // Request stop and wait for the thread to finish , jthread automatically joins on assignment or destruction
    thread_.request_stop();
    thread_ = std::jthread();  // Reset to empty jthread (this waits for old one to finish)

    running_.store(false, std::memory_order_release);
}

bool DecodeThread::isEndOfStream() const {
    return endOfStream_.load(std::memory_order_acquire);
}

bool DecodeThread::isRunning() const {
    return running_.load(std::memory_order_acquire);
}

void DecodeThread::decodeLoop(std::stop_token stopToken) {
    if (!decoder_ || !ringBuffer_) {
        return;
    }

    // Allocate a local buffer for decoded frames , size: chunkFrames * channels * sizeof(int16_t)
    const auto channels = static_cast<std::size_t>(decoder_->channels());
    const std::size_t bufferSize = chunkFrames_ * channels;
    std::vector<int16_t> decodeBuffer(bufferSize);

    while (!stopToken.stop_requested()) {

        const std::size_t samplesNeeded = chunkFrames_ * channels;
        const std::size_t spaceAvailable = ringBuffer_->availableToWrite();

        if (spaceAvailable < samplesNeeded) { // Check if the ring buffer has enough space
            std::this_thread::sleep_for(Config::DECODE_THREAD_SLEEP_MS);
            continue;
        }

        const std::span<int16_t> outputSpan(decodeBuffer.data(), bufferSize);
        const std::size_t framesDecoded = decoder_->decodeFrames(outputSpan, chunkFrames_);

        if (framesDecoded == 0) {
            // End of stream reached
            endOfStream_.store(true, std::memory_order_release);
            break;
        }

        // Write  decoded samples to  ring buffer
        const std::size_t samplesToWrite = framesDecoded * channels;
        const std::size_t samplesWritten = ringBuffer_->write(decodeBuffer.data(), samplesToWrite);

    }

    running_.store(false, std::memory_order_release);
}
