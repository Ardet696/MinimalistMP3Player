#include "SpectrumAnalyzer.h"
#include <cmath>
#include <algorithm>

SpectrumAnalyzer::SpectrumAnalyzer(int barCount)
    : barCount_(barCount)
    , hannWindow_(kFftSize)
    , bars_(barCount, 0.0f)
    , spectrumMagnitudes_(kMagnitudeCount, 0.0f)
    , fftData_(kFftSize)
    , magnitudes_(kMagnitudeCount, 0.0f)
{
    staging_[0].assign(kFftSize, 0.0f);
    staging_[1].assign(kFftSize, 0.0f);
    for (int i = 0; i < kFftSize; i++) {
        hannWindow_[i] = 0.5 * (1.0 - std::cos(2.0 * M_PI * i / (kFftSize - 1)));
    }
}

void SpectrumAnalyzer::feed(const int16_t* samples, std::size_t sampleCount, int channels) {
    if (channels <= 0) return;
    const std::size_t monoCount = sampleCount / static_cast<std::size_t>(channels);
    if (monoCount == 0) return;

    const std::size_t n = std::min(monoCount, static_cast<std::size_t>(kFftSize));
    const int back = stagingBack_;
    float* dst = staging_[back].data();

    for (std::size_t i = 0; i < n; i++) {
        float sum = 0.0f;
        for (int ch = 0; ch < channels; ch++)
            sum += static_cast<float>(samples[i * channels + ch]);
        dst[i] = sum / static_cast<float>(channels);
    }

    stagingLen_[back] = n;
    stagingFront_.store(back, std::memory_order_release);
    stagingGen_.fetch_add(1, std::memory_order_release);
    stagingBack_ = 1 - back;
}

void SpectrumAnalyzer::updateFromStaging() const {
    const std::uint32_t gen = stagingGen_.load(std::memory_order_acquire);
    if (gen == lastGen_) return;
    lastGen_ = gen;

    const int front = stagingFront_.load(std::memory_order_acquire);
    if (front < 0) return;

    const std::size_t len = std::min(stagingLen_[front], static_cast<std::size_t>(kFftSize));
    const float* src = staging_[front].data();

    std::fill(fftData_.begin(), fftData_.end(), Fft::cd(0.0, 0.0));
    for (std::size_t i = 0; i < len; i++) {
        fftData_[i] = src[i] * hannWindow_[i];
    }

    Fft::compute(fftData_);

    for (int i = 0; i < kMagnitudeCount; i++)
        magnitudes_[i] = static_cast<float>(std::abs(fftData_[i])) / kFftSize;

    const double logRatio = std::log(static_cast<double>(kMagnitudeCount));

    std::fill(bars_.begin(), bars_.end(), 0.0f);
    for (int b = 0; b < barCount_; b++) {
        const double t0 = static_cast<double>(b)     / barCount_;
        const double t1 = static_cast<double>(b + 1) / barCount_;
        int start = static_cast<int>(std::exp(t0 * logRatio));  // 1 * exp(t * ln(512))
        int end   = static_cast<int>(std::exp(t1 * logRatio));
        if (end <= start) end = start + 1;
        if (end > kMagnitudeCount) end = kMagnitudeCount;

        float sum = 0.0f;
        for (int i = start; i < end; i++)
            sum += magnitudes_[i];
        bars_[b] = sum / static_cast<float>(end - start);
    }

    const float maxBar = *std::max_element(bars_.begin(), bars_.end());
    if (maxBar > 0.0f) {
        for (float& v : bars_)
            v = std::clamp(v / maxBar, 0.0f, 1.0f);
    }

    spectrumMagnitudes_ = magnitudes_;
}

std::vector<float> SpectrumAnalyzer::getBars() const {
    updateFromStaging();
    return bars_;
}

std::vector<float> SpectrumAnalyzer::getSpectrumMagnitudes() const {
    updateFromStaging();
    return spectrumMagnitudes_;
}
