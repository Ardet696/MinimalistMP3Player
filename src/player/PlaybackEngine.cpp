#include "PlaybackEngine.h"
#include "../decode/Mp3Decoder.h"
#include "../audio/SdlAudioSink.h"
#include "../util/RingBuffer.h"
#include "../config/Config.h"
#include "../events/NotificationBus.h"
#include "DecodeThread.h"
#include <algorithm>
constexpr int SPECTRUM_BARS = 40;
PlaybackEngine::PlaybackEngine(NotificationBus* bus)
    : state_(State::Stopped)
    ,sampleRate_(0)
    ,channels_(0)
    , silentCallbacks_(0)
    , spectrumAnalyzer_(SPECTRUM_BARS)
    , samplesPlayed_(0)
    , totalSamples_(0)
    , bus_(bus)
{
}

PlaybackEngine::~PlaybackEngine() {
    stop();
}

bool PlaybackEngine::load(const std::filesystem::path& mp3File) {
    std::lock_guard lock(mutex_);

    if (state_.load(std::memory_order_acquire) != State::Stopped) {
        stopPlayback();
    }

    decoder_ = std::make_unique<Mp3Decoder>();
    if (!decoder_->open(mp3File)) {
        if (bus_) bus_->push("Failed to open: " + mp3File.filename().string(), NotifyLevel::Error);
        decoder_.reset();
        return false;
    }

    // Store file info
    currentFile_ = mp3File;
    sampleRate_ = decoder_->sampleRate();
    channels_ = decoder_->channels();
    totalSamples_ = decoder_->totalSamples();
    samplesPlayed_.store(0, std::memory_order_release);
    bpmDetector_.reset();

    const std::size_t ringBufferCapacity = Config::RING_BUFFER_SIZE_SECONDS * sampleRate_ * channels_;
    ringBuffer_ = std::make_unique<RingBuffer<int16_t>>(ringBufferCapacity);

    decodeThread_ = std::make_unique<DecodeThread>();

    AudioFormat fmt;
    fmt.sampleRate = sampleRate_;
    fmt.channels = channels_;

    auto audioProvider = [this](int16_t* dst, std::size_t framesRequested) -> std::size_t {
        const std::size_t samplesRequested = framesRequested * channels_;
        const std::size_t samplesRead = ringBuffer_->read(dst, samplesRequested);
        const std::size_t framesRead = samplesRead / channels_;

        if (framesRead == 0) {
            silentCallbacks_.fetch_add(1, std::memory_order_relaxed);
        } else {
            silentCallbacks_.store(0, std::memory_order_relaxed);
            samplesPlayed_.fetch_add(samplesRead, std::memory_order_relaxed);
            spectrumAnalyzer_.feed(dst, samplesRead, channels_);
            bpmDetector_.feed(dst, samplesRead, channels_, sampleRate_);
        }

        return framesRead;
    };

    sink_ = std::make_unique<SdlAudioSink>(bus_);
    if (!sink_->open(fmt, audioProvider, outputDeviceName_)) {
        if (bus_) bus_->push("Failed to open audio sink", NotifyLevel::Error);
        decoder_->close();
        decoder_.reset();
        ringBuffer_.reset();
        decodeThread_.reset();
        sink_.reset();
        return false;
    }

    sink_->setVolume(volume_.load(std::memory_order_relaxed));
    state_.store(State::Stopped, std::memory_order_release);
    return true;
}

void PlaybackEngine::play() {
    std::lock_guard lock(mutex_);

    const State currentState = state_.load(std::memory_order_acquire);

    if (currentState == State::Playing) {
        return;
    }

    if (!decoder_ || !sink_) {
        if (bus_) bus_->push("No file loaded", NotifyLevel::Error);
        return;
    }

    if (currentState == State::Stopped) {
        if (!startPlayback()) {
            if (bus_) bus_->push("Failed to start playback", NotifyLevel::Error);
            return;
        }
    } else if (currentState == State::Paused) {
        sink_->start();
    }

    state_.store(State::Playing, std::memory_order_release);
}

void PlaybackEngine::pause() {
    std::lock_guard lock(mutex_);

    if (state_.load(std::memory_order_acquire) != State::Playing) {
        return;
    }

    if (sink_) {
        sink_->stop();
    }

    state_.store(State::Paused, std::memory_order_release);
}

void PlaybackEngine::stop() {
    std::lock_guard lock(mutex_);
    stopPlayback();
}

bool PlaybackEngine::isPlaying() const {
    return state_.load(std::memory_order_acquire) == State::Playing;
}

bool PlaybackEngine::isPaused() const {
    return state_.load(std::memory_order_acquire) == State::Paused;
}

bool PlaybackEngine::isStopped() const {
    return state_.load(std::memory_order_acquire) == State::Stopped;
}

PlaybackEngine::State PlaybackEngine::getState() const {
    return state_.load(std::memory_order_acquire);
}

bool PlaybackEngine::hasReachedEndOfStream() const {
    return silentCallbacks_.load(std::memory_order_acquire) >= Config::END_OF_STREAM_THRESHOLD;
}

std::filesystem::path PlaybackEngine::getCurrentFile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentFile_;
}

int PlaybackEngine::getSampleRate() const {
    return sampleRate_;
}

int PlaybackEngine::getChannels() const {
    return channels_;
}

float PlaybackEngine::getBpmLoadFactor() const {
    return bpmDetector_.getLoadFactor();
}

float PlaybackEngine::getPlaybackProgress() const {
    if (totalSamples_ == 0) return 0.0f;
    auto played = samplesPlayed_.load(std::memory_order_relaxed);
    float progress = static_cast<float>(played) / static_cast<float>(totalSamples_);
    return std::min(progress, 1.0f);
}

bool PlaybackEngine::startPlayback() {

    if (!decoder_ || !ringBuffer_ || !decodeThread_ || !sink_) {
        return false;
    }

    silentCallbacks_.store(0, std::memory_order_release);

    decodeThread_->start(decoder_.get(), ringBuffer_.get(), Config::DECODE_CHUNK_FRAMES);

    const std::size_t targetSamples = static_cast<std::size_t>(
        ringBuffer_->availableToWrite() * Config::RING_BUFFER_PREFILL_PERCENT
    );
    while (ringBuffer_->availableToRead() < targetSamples) {
        if (decodeThread_->isEndOfStream()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    sink_->start();

    return true;
}

void PlaybackEngine::setVolume(int percent) {
    volume_.store(std::clamp(percent, 0, 100), std::memory_order_relaxed);
    std::lock_guard lock(mutex_);
    if (sink_) sink_->setVolume(percent);
}

int PlaybackEngine::getVolume() const {
    return volume_.load(std::memory_order_relaxed);
}

void PlaybackEngine::setOutputDevice(const std::string& deviceName) {
    std::lock_guard lock(mutex_);
    outputDeviceName_ = deviceName;
}

std::string PlaybackEngine::getOutputDevice() const {
    std::lock_guard lock(mutex_);
    return outputDeviceName_;
}

std::vector<std::string> PlaybackEngine::listOutputDevices() {
    return SdlAudioSink::listOutputDevices();
}

void PlaybackEngine::stopPlayback() {
    // Must be called with mutex_ held

    if (sink_) {
        sink_->stop();
        sink_->close();
        sink_.reset();
    }

    if (decodeThread_) {
        decodeThread_->stop();
        decodeThread_.reset();
    }
    if (decoder_) {
        decoder_->close();
        decoder_.reset();
    }
    ringBuffer_.reset();

    state_.store(State::Stopped, std::memory_order_release);
    currentFile_.clear();
    sampleRate_ = 0;
    channels_ = 0;
    totalSamples_ = 0;
    samplesPlayed_.store(0, std::memory_order_release);
}
