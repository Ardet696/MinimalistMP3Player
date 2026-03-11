#include "SpectrumAnalyzer.h"
#include <cmath>
#include <algorithm>

constexpr int FFT_SIZE = 1024;
constexpr int MAGNITUDE_COUNT = FFT_SIZE / 2;  // 512 usable frequency bins

SpectrumAnalyzer::SpectrumAnalyzer(int barCount)
    : barCount_(barCount)
    , bars_(barCount, 0.0f)
    , spectrumMagnitudes_(MAGNITUDE_COUNT, 0.0f)
    , mono_(FFT_SIZE)
    , fftData_(FFT_SIZE)
    , magnitudes_(MAGNITUDE_COUNT)
    , newBars_(barCount, 0.0f)
{
}

void SpectrumAnalyzer::feed(const int16_t* samples, std::size_t sampleCount, int channels) {
    // Mix down to mono
    std::size_t monoCount = sampleCount / channels;
    if (monoCount == 0) return;

    // Reuse pre-allocated buffer, resize only if needed (no-op if already large enough)
    if (mono_.size() < monoCount) mono_.resize(monoCount);

    for (std::size_t i = 0; i < monoCount; i++) {
        float sum = 0.0f;
        for (int ch = 0; ch < channels; ch++)
            sum += static_cast<float>(samples[i * channels + ch]);
        mono_[i] = sum / channels;
    }

    // Prepare FFT input: pad/truncate to FFT_SIZE, apply Hann window
    std::fill(fftData_.begin(), fftData_.end(), Fft::cd(0.0, 0.0));
    std::size_t len = std::min(monoCount, static_cast<std::size_t>(FFT_SIZE));
    for (std::size_t i = 0; i < len; i++) {
        double window = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (len - 1)));
        fftData_[i] = mono_[i] * window;
    }

    Fft::compute(fftData_);

    // Compute magnitudes from first half (symmetric for real input)
    for (int i = 0; i < MAGNITUDE_COUNT; i++)
        magnitudes_[i] = static_cast<float>(std::abs(fftData_[i])) / FFT_SIZE;

    // Group into log-scaled bars using true exponential (log-frequency) mapping.
    // Starts at bin 1 (~43 Hz at 44100/1024) to skip bin 0 (DC component), which
    // is not a musical frequency and inflates the first bars artificially.
    // Each bar covers an equal number of octaves, matching human pitch perception.
    const double logRatio = std::log(static_cast<double>(MAGNITUDE_COUNT)); // ln(512/1)

    std::fill(newBars_.begin(), newBars_.end(), 0.0f);
    for (int b = 0; b < barCount_; b++) {
        const double t0 = static_cast<double>(b)     / barCount_;
        const double t1 = static_cast<double>(b + 1) / barCount_;
        int start = static_cast<int>(std::exp(t0 * logRatio));  // 1 * exp(t * ln(512))
        int end   = static_cast<int>(std::exp(t1 * logRatio));
        if (end <= start) end = start + 1;
        if (end > MAGNITUDE_COUNT) end = MAGNITUDE_COUNT;

        float sum = 0.0f;
        for (int i = start; i < end; i++)
            sum += magnitudes_[i];
        newBars_[b] = sum / (end - start);
    }

    // Normalize bars to 0.0-1.0 range
    float maxBar = *std::max_element(newBars_.begin(), newBars_.end());
    if (maxBar > 0.0f) {
        for (float& v : newBars_)
            v = std::clamp(v / maxBar, 0.0f, 1.0f);
    }

    std::lock_guard lock(mutex_);
    std::copy(newBars_.begin(), newBars_.end(), bars_.begin());
    std::copy(magnitudes_.begin(), magnitudes_.end(), spectrumMagnitudes_.begin());
}

std::vector<float> SpectrumAnalyzer::getBars() const {
    std::lock_guard lock(mutex_);
    return bars_;
}

std::vector<float> SpectrumAnalyzer::getSpectrumMagnitudes() const
{
    std::lock_guard lock(mutex_);
    return spectrumMagnitudes_;
}
