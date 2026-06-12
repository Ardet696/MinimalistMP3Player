#include <catch2/catch_all.hpp>
#include "../../src/decode/SpectrumAnalyzer.h"
#include <cmath>
#include <vector>

static const double PI = std::acos(-1.0);

TEST_CASE("SpectrumAnalyzer silence produces zero bars", "[spectrum]") {
    SpectrumAnalyzer sa(40);
    std::vector<int16_t> silence(2048, 0);
    sa.feed(silence.data(), silence.size(), 2);

    auto bars = sa.getBars();
    CHECK(bars.size() == 40);
    for (float b : bars)
        CHECK(b == 0.0f);
}

TEST_CASE("SpectrumAnalyzer output is in [0, 1] range", "[spectrum]") {
    SpectrumAnalyzer sa(40);

    // Feed a 440Hz sine at 44100 sample rate, stereo
    constexpr int N = 2048;
    constexpr int sampleRate = 44100;
    constexpr double freq = 440.0;
    std::vector<int16_t> samples(N * 2); // stereo
    for (int i = 0; i < N; i++) {
        auto val = static_cast<int16_t>(16000 * std::sin(2.0 * PI * freq * i / sampleRate));
        samples[i * 2] = val;
        samples[i * 2 + 1] = val;
    }
    sa.feed(samples.data(), samples.size(), 2);

    auto bars = sa.getBars();
    CHECK(bars.size() == 40);
    for (float b : bars) {
        CHECK(b >= 0.0f);
        CHECK(b <= 1.0f);
    }

    // At least one bar should have energy
    float maxBar = *std::max_element(bars.begin(), bars.end());
    CHECK(maxBar > 0.0f);
}

TEST_CASE("SpectrumAnalyzer returns correct bar count", "[spectrum]") {
    SpectrumAnalyzer sa10(10);
    SpectrumAnalyzer sa80(80);

    std::vector<int16_t> noise(2048);
    for (auto& s : noise) s = static_cast<int16_t>(rand() % 10000 - 5000);

    sa10.feed(noise.data(), noise.size(), 1);
    sa80.feed(noise.data(), noise.size(), 1);

    CHECK(sa10.getBars().size() == 10);
    CHECK(sa80.getBars().size() == 80);
    CHECK(sa10.getSpectrumMagnitudes().size() == 512);
    CHECK(sa80.getSpectrumMagnitudes().size() == 512);
}

TEST_CASE("SpectrumAnalyzer handles small input", "[spectrum]") {
    SpectrumAnalyzer sa(40);
    // Feed fewer samples than FFT_SIZE (1024) — should still work (zero-padded)
    std::vector<int16_t> small(64, 1000);
    sa.feed(small.data(), small.size(), 1);
    auto bars = sa.getBars();
    CHECK(bars.size() == 40);
}
