#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>
#include <vector>

#include "../../src/decode/BpmDetector.h"

static constexpr int    SAMPLE_RATE   = 44100;
static constexpr int    CHANNELS      = 2;
static constexpr size_t CHUNK_FRAMES  = 1152;
static constexpr size_t CHUNK_SAMPLES = CHUNK_FRAMES * CHANNELS;
static constexpr int    ITERATIONS    = 10000;

static std::vector<int16_t> makeBassChunk(int iteration) {
    std::vector<int16_t> buf(CHUNK_SAMPLES);
    for (size_t f = 0; f < CHUNK_FRAMES; ++f) {
        float t = static_cast<float>(iteration * CHUNK_FRAMES + f) / SAMPLE_RATE;
        // 80 Hz bass sine with occasional transient to trigger beat detection
        float amp = (f % 256 < 10) ? 28000.f : 12000.f;
        int16_t s = static_cast<int16_t>(amp * std::sin(2.f * 3.14159265f * 80.f * t));
        buf[f * CHANNELS]     = s;
        buf[f * CHANNELS + 1] = s;
    }
    return buf;
}

int main() {
    BpmDetector detector;
    std::vector<double> times;
    times.reserve(ITERATIONS);

    for (int i = 0; i < ITERATIONS; ++i) {
        auto chunk = makeBassChunk(i);
        auto t0 = std::chrono::steady_clock::now();
        detector.feed(chunk.data(), CHUNK_SAMPLES, CHANNELS, SAMPLE_RATE);
        auto t1 = std::chrono::steady_clock::now();
        times.push_back(
            std::chrono::duration<double, std::nano>(t1 - t0).count());
    }

    double mean = std::accumulate(times.begin(), times.end(), 0.0) / ITERATIONS;
    double maxT = *std::max_element(times.begin(), times.end());
    double budget = 1e9 * static_cast<double>(CHUNK_FRAMES) / SAMPLE_RATE;

    std::printf("[BpmDetector::feed() — audio callback path]\n");
    std::printf("  mean : %.1f ns\n", mean);
    std::printf("  max  : %.1f ns\n", maxT);
    std::printf("  budget at 44100 Hz / %zu-frame chunks: %.0f ns\n", CHUNK_FRAMES, budget);
    std::printf("  mean as %% of budget: %.1f%%\n",
        100.0 * mean / budget);

    return 0;
}
