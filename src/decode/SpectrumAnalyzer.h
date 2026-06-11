#ifndef MP3PLAYER_SPECTRUMANALYZER_H
#define MP3PLAYER_SPECTRUMANALYZER_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "Fft.h"

class SpectrumAnalyzer {
public:
    explicit SpectrumAnalyzer(int barCount);

    // Called from the audio thread: stage PCM for later analysis.
    // Wait-free for the caller: no FFT, no mutex, no allocation.
    void feed(const int16_t* samples, std::size_t sampleCount, int channels);

    // Called from the UI thread: recompute from the latest staged audio
    // (only when new data has arrived) and return the result.
    std::vector<float> getBars() const;
    std::vector<float> getSpectrumMagnitudes() const; // 512 for audiovisualizer

    int barCount() const { return barCount_; }

private:
    static constexpr int kFftSize = 1024;
    static constexpr int kMagnitudeCount = kFftSize / 2;  // 512 usable frequency bins

    void updateFromStaging() const;

    int barCount_;

    // Audio -> UI handoff: double-buffered, lock-free for the audio thread.
    std::vector<float> staging_[2];
    std::size_t stagingLen_[2]{0, 0};
    std::atomic<int> stagingFront_{-1};
    std::atomic<std::uint32_t> stagingGen_{0};
    int stagingBack_ = 0;  // audio thread only

    std::vector<double> hannWindow_;

    // Results and scratch (UI thread only; mutable cache behind const getters).
    mutable std::uint32_t lastGen_ = 0;
    mutable std::vector<float> bars_;
    mutable std::vector<float> spectrumMagnitudes_;
    mutable std::vector<Fft::cd> fftData_;
    mutable std::vector<float> magnitudes_;
};

#endif // MP3PLAYER_SPECTRUMANALYZER_H
