#include "BpmDetector.h"

#include <algorithm>

static constexpr float TARGET_RATE    = 500.f;  // Hz after downsampling
static constexpr float LPF_CUTOFF     = 150.f;  // Hz  (kick/bass isolation)
static constexpr float ONSET_FACTOR   = 1.5f;   // energy must exceed 1.5× local avg
static constexpr float EMA_ALPHA      = 0.15f;  // load-factor smoothing

BpmDetector::BpmDetector() {
    history_.fill(0.f);
    intervals_.fill(0.0);
}

void BpmDetector::reset() {
    lpfState_         = 0.f;
    alpha_            = 0.f;
    cachedRate_       = 0;
    downsampleStep_   = 1;
    downsamplePos_    = 0;
    energyAccum_      = 0.f;
    energyCount_      = 0;
    historyPos_       = 0;
    totalSamples_     = 0;
    lastBeatSample_   = 0;
    intervalPos_      = 0;
    intervalsCollected_ = 0;
    smoothedLoadFactor_ = 0.5f;
    history_.fill(0.f);
    intervals_.fill(0.0);
    loadFactor_.store(0.5f, std::memory_order_relaxed);
}

float BpmDetector::getLoadFactor() const {
    return loadFactor_.load(std::memory_order_relaxed);
}

void BpmDetector::feed(const int16_t* samples, std::size_t count, int channels, int sampleRate) {
    if (count == 0 || channels <= 0 || sampleRate <= 0) return;

    // Recompute filter / downsample parameters if sample rate changed
    if (sampleRate != cachedRate_) {
        cachedRate_     = sampleRate;
        const float w   = 2.f * 3.14159265f * LPF_CUTOFF;
        alpha_          = w / (static_cast<float>(sampleRate) + w);
        downsampleStep_ = std::max(1, static_cast<int>(sampleRate / TARGET_RATE));
        downsamplePos_  = 0;
    }

    const std::size_t frameCount = count / static_cast<std::size_t>(channels);

    for (std::size_t f = 0; f < frameCount; ++f) {
        // 1. Mix channels to mono
        float mono = 0.f;
        for (int c = 0; c < channels; ++c) {
            mono += static_cast<float>(samples[f * static_cast<std::size_t>(channels) + c]);
        }
        mono /= static_cast<float>(channels);

        // 2. IIR low-pass filter (exponential moving average)
        lpfState_ = alpha_ * mono + (1.f - alpha_) * lpfState_;

        // 3. Downsample: keep every downsampleStep_-th filtered sample
        ++downsamplePos_;
        if (downsamplePos_ < downsampleStep_) continue;
        downsamplePos_ = 0;

        ++totalSamples_;
        const float s = lpfState_;

        // 4. Accumulate short-time energy
        energyAccum_ += s * s;
        ++energyCount_;

        if (energyCount_ < ENERGY_WINDOW) continue;

        // Energy window complete
        const float E = energyAccum_ / static_cast<float>(ENERGY_WINDOW);
        energyAccum_ = 0.f;
        energyCount_ = 0;

        // 5. Compute local average from history
        float histSum = 0.f;
        for (float v : history_) histSum += v;
        const float localAvg = histSum / static_cast<float>(HISTORY_SIZE);

        // Store energy in circular history
        history_[historyPos_] = E;
        historyPos_ = (historyPos_ + 1) % HISTORY_SIZE;

        // Beat onset: energy significantly above local average and past debounce
        const uint64_t sinceLastBeat = totalSamples_ - lastBeatSample_;
        if (E > ONSET_FACTOR * localAvg && sinceLastBeat > static_cast<uint64_t>(DEBOUNCE)) {
            // 6. Record inter-beat interval in seconds
            if (lastBeatSample_ > 0) {
                const double intervalSec = static_cast<double>(sinceLastBeat) / TARGET_RATE;
                intervals_[intervalPos_] = intervalSec;
                intervalPos_ = (intervalPos_ + 1) % INTERVAL_COUNT;
                if (intervalsCollected_ < INTERVAL_COUNT) ++intervalsCollected_;

                // 7. Estimate BPM from collected intervals
                if (intervalsCollected_ >= 2) {
                    double sum = 0.0;
                    for (int i = 0; i < intervalsCollected_; ++i) sum += intervals_[i];
                    const double meanInterval = sum / static_cast<double>(intervalsCollected_);
                    const float bpm = static_cast<float>(60.0 / meanInterval);

                    // Map BPM to loadFactor: 60 BPM → 0.2, 180 BPM → 1.0
                    const float raw = (bpm - 60.f) / 120.f;
                    const float clamped = std::clamp(raw, 0.2f, 1.0f);

                    // Smooth with EMA to avoid abrupt jumps
                    smoothedLoadFactor_ = EMA_ALPHA * clamped + (1.f - EMA_ALPHA) * smoothedLoadFactor_;
                    loadFactor_.store(smoothedLoadFactor_, std::memory_order_relaxed);
                }
            }

            lastBeatSample_ = totalSamples_;
        }
    }
}
