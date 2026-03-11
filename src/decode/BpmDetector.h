#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

/**
 * BpmDetector - Real-time energy based beat detector.
 *
 * Pipeline (per feed() call):
 *   PCM → mono mix → IIR low-pass (~150 Hz) -> downsample to 500 Hz
 *   → short time energy window -> onset detection -> beat intervals → BPM
 *   → loadFactor (0.0–1.0, atomic)
 *
 * Thread safety:
 *   feed() and reset() must be called from the audio thread only.
 *   getLoadFactor() may be called from any thread (atomic read).
 */
class BpmDetector {
public:
    BpmDetector();

    void feed(const int16_t* samples, std::size_t count, int channels, int sampleRate);
    void reset();
    float getLoadFactor() const;

private:
    // LPF state
    float lpfState_   = 0.f;
    float alpha_      = 0.f;
    int   cachedRate_ = 0;

    // Downsampling
    int downsampleStep_ = 1;
    int downsamplePos_  = 0;

    // Short-time energy window (~50 ms = 25 samples at 500 Hz)
    static constexpr int ENERGY_WINDOW = 25;
    float energyAccum_ = 0.f;
    int   energyCount_ = 0;

    // Energy history (43 windows ≈ 2.15 s)
    static constexpr int HISTORY_SIZE = 43;
    std::array<float, HISTORY_SIZE> history_{};
    int historyPos_ = 0;

    // Beat timing (counted in downsampled samples at 500 Hz)
    uint64_t totalSamples_   = 0;
    uint64_t lastBeatSample_ = 0;
    static constexpr int DEBOUNCE = 125; // 250 ms at 500 Hz

    // Inter-beat interval history (last 8 beats)
    static constexpr int INTERVAL_COUNT = 8;
    std::array<double, INTERVAL_COUNT> intervals_{};
    int intervalPos_        = 0;
    int intervalsCollected_ = 0;

    // Smoothed load factor output
    float smoothedLoadFactor_ = 0.5f;

    // Atomic output (read by UI thread)
    std::atomic<float> loadFactor_{0.5f};
};
