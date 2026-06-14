#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <vector>

#include "../../src/decode/SpectrumAnalyzer.h"

static constexpr int    SAMPLE_RATE   = 44100;
static constexpr int    CHANNELS      = 2;
static constexpr size_t CHUNK_FRAMES  = 1152;
static constexpr size_t CHUNK_SAMPLES = CHUNK_FRAMES * CHANNELS;
static constexpr int    BAR_COUNT     = 40;
static constexpr int    ITERATIONS    = 2000;

static std::vector<int16_t> makeSineChunk(float freq, int iteration) {
    std::vector<int16_t> buf(CHUNK_SAMPLES);
    for (size_t f = 0; f < CHUNK_FRAMES; ++f) {
        float t = static_cast<float>(iteration * CHUNK_FRAMES + f) / SAMPLE_RATE;
        int16_t s = static_cast<int16_t>(16000.f * std::sin(2.f * 3.14159265f * freq * t));
        buf[f * CHANNELS]     = s;
        buf[f * CHANNELS + 1] = s;
    }
    return buf;
}

int main() {
    SpectrumAnalyzer analyzer(BAR_COUNT);

    // --- feed() cost (audio callback path) ---
    {
        std::vector<double> times;
        times.reserve(ITERATIONS);

        for (int i = 0; i < ITERATIONS; ++i) {
            auto chunk = makeSineChunk(440.f, i);
            auto t0 = std::chrono::steady_clock::now();
            analyzer.feed(chunk.data(), CHUNK_SAMPLES, CHANNELS);
            auto t1 = std::chrono::steady_clock::now();
            times.push_back(
                std::chrono::duration<double, std::nano>(t1 - t0).count());
        }

        double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
        double maxT = *std::max_element(times.begin(), times.end());
        std::printf("[SpectrumAnalyzer::feed()  — audio callback path]\n");
        std::printf("  mean : %.1f ns\n", mean);
        std::printf("  max  : %.1f ns\n", maxT);
        std::printf("  budget at 44100 Hz / %zu-frame chunks: %.0f ns\n\n",
            CHUNK_FRAMES,
            1e9 * static_cast<double>(CHUNK_FRAMES) / SAMPLE_RATE);
    }

    // --- getBars() cost (UI thread, runs every 33 ms) ---
    {
        // Pre-fill staging with fresh data so every call does real FFT work
        std::vector<double> times;
        times.reserve(ITERATIONS);

        for (int i = 0; i < ITERATIONS; ++i) {
            // feed new data so stagingGen_ advances and FFT is not skipped
            auto chunk = makeSineChunk(440.f + i, i);
            analyzer.feed(chunk.data(), CHUNK_SAMPLES, CHANNELS);

            auto t0 = std::chrono::steady_clock::now();
            auto bars = analyzer.getBars();
            auto t1 = std::chrono::steady_clock::now();
            (void)bars;

            times.push_back(
                std::chrono::duration<double, std::micro>(t1 - t0).count());
        }

        double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
        double maxT = *std::max_element(times.begin(), times.end());
        std::printf("[SpectrumAnalyzer::getBars() — UI thread, 30 fps]\n");
        std::printf("  mean : %.2f µs\n", mean);
        std::printf("  max  : %.2f µs\n", maxT);
        std::printf("  budget per frame at 30 fps: 33333 µs\n\n");
    }

    return 0;
}
