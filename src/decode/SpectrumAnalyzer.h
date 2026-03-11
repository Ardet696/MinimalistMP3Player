#ifndef MP3PLAYER_SPECTRUMANALYZER_H
#define MP3PLAYER_SPECTRUMANALYZER_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <mutex>
#include "Fft.h"

class SpectrumAnalyzer {
public:
    explicit SpectrumAnalyzer(int barCount);

    // Called from audio thread: feed PCM samples and recompute bars
    void feed(const int16_t* samples, std::size_t sampleCount, int channels);
    std::vector<float> getBars() const;

    int barCount() const { return barCount_; }
    std::vector<float> getSpectrumMagnitudes() const; // 512 for audiovisualizer
private:
    int barCount_;
    std::vector<float> bars_;
    std::vector<float> spectrumMagnitudes_;
    mutable std::mutex mutex_;

    // Pre-allocated work buffers to avoid heap allocations on the audio thread
    std::vector<float> mono_;
    std::vector<Fft::cd> fftData_;
    std::vector<float> magnitudes_;
    std::vector<float> newBars_;
};

#endif // MP3PLAYER_SPECTRUMANALYZER_H
