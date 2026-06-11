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
    running_.store(true, std::memory_order_release);

    thread_ = std::jthread([this](std::stop_token stopToken) {
        decodeLoop(std::move(stopToken));
    });
}

void DecodeThread::stop() {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    thread_.request_stop();
    thread_ = std::jthread();

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

    const auto channels = static_cast<std::size_t>(decoder_->channels());
    const std::size_t bufferSize = chunkFrames_ * channels;
    std::vector<int16_t> decodeBuffer(bufferSize);

    while (!stopToken.stop_requested()) {

        const std::size_t samplesNeeded = chunkFrames_ * channels;
        const std::size_t spaceAvailable = ringBuffer_->availableToWrite();

        if (spaceAvailable < samplesNeeded) {
            std::this_thread::sleep_for(Config::DECODE_THREAD_SLEEP_MS);
            continue;
        }

        const std::span<int16_t> outputSpan(decodeBuffer.data(), bufferSize);
        const std::size_t framesDecoded = decoder_->decodeFrames(outputSpan, chunkFrames_);

        if (framesDecoded == 0) {
            endOfStream_.store(true, std::memory_order_release);
            break;
        }

        const std::size_t samplesToWrite = framesDecoded * channels;
        ringBuffer_->write(decodeBuffer.data(), samplesToWrite);

    }

    running_.store(false, std::memory_order_release);
}
