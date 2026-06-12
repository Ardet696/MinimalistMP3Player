#include <catch2/catch_all.hpp>
#include "../../src/decode/BpmDetector.h"
#include <vector>
#include <cmath>

TEST_CASE("BpmDetector initial state", "[bpm]") {
    BpmDetector bd;
    CHECK(bd.getLoadFactor() == Catch::Approx(0.5f));
}

TEST_CASE("BpmDetector silence keeps default load factor", "[bpm]") {
    BpmDetector bd;
    std::vector<int16_t> silence(44100 * 2, 0); // 1 second stereo silence
    bd.feed(silence.data(), silence.size(), 2, 44100);
    CHECK(bd.getLoadFactor() == Catch::Approx(0.5f));
}

TEST_CASE("BpmDetector load factor stays in valid range", "[bpm]") {
    BpmDetector bd;
    constexpr int sampleRate = 44100;
    constexpr int channels = 2;

    // Feed random-ish data to push the detector around
    std::vector<int16_t> data(sampleRate * channels * 4); // 4 seconds
    for (size_t i = 0; i < data.size(); i++)
        data[i] = static_cast<int16_t>((i * 7919) % 32000 - 16000);

    bd.feed(data.data(), data.size(), channels, sampleRate);
    float lf = bd.getLoadFactor();
    CHECK(lf >= 0.0f);
    CHECK(lf <= 1.0f);
}

TEST_CASE("BpmDetector synthetic 120 BPM clicks", "[bpm]") {
    BpmDetector bd;
    constexpr int sampleRate = 44100;
    constexpr int channels = 1;
    constexpr double bpm = 120.0;
    constexpr int samplesPerBeat = static_cast<int>(60.0 / bpm * sampleRate);

    // Generate 10 seconds of audio with loud clicks at 120 BPM
    constexpr int totalSamples = sampleRate * 10;
    std::vector<int16_t> audio(totalSamples, 0);
    for (int i = 0; i < totalSamples; i += samplesPerBeat) {
        // Short loud click (50 samples)
        for (int j = 0; j < 50 && (i + j) < totalSamples; j++)
            audio[i + j] = 30000;
    }

    // Feed in chunks like the real audio callback would
    constexpr int chunkSize = 2048;
    for (int offset = 0; offset < totalSamples; offset += chunkSize) {
        int remaining = std::min(chunkSize, totalSamples - offset);
        bd.feed(audio.data() + offset, remaining, channels, sampleRate);
    }

    // After enough data, load factor should have moved from default
    // 120 BPM maps to (120-60)/120 = 0.5, but with smoothing it converges slowly
    float lf = bd.getLoadFactor();
    CHECK(lf >= 0.2f);
    CHECK(lf <= 1.0f);
}

TEST_CASE("BpmDetector reset restores initial state", "[bpm]") {
    BpmDetector bd;
    std::vector<int16_t> noise(44100);
    for (auto& s : noise) s = static_cast<int16_t>(rand() % 20000 - 10000);
    bd.feed(noise.data(), noise.size(), 1, 44100);

    bd.reset();
    CHECK(bd.getLoadFactor() == Catch::Approx(0.5f));
}

TEST_CASE("BpmDetector handles edge inputs", "[bpm]") {
    BpmDetector bd;

    SECTION("zero count is no-op") {
        bd.feed(nullptr, 0, 2, 44100);
        CHECK(bd.getLoadFactor() == Catch::Approx(0.5f));
    }

    SECTION("single sample") {
        int16_t sample = 1000;
        bd.feed(&sample, 1, 1, 44100);
        CHECK(bd.getLoadFactor() == Catch::Approx(0.5f));
    }
}
