#include <catch2/catch_all.hpp>
#include "../../src/decode/Fft.h"
#include <cmath>

static const double PI = std::acos(-1.0);

TEST_CASE("FFT DC signal has energy only in bin 0", "[fft]") {
    std::vector<Fft::cd> data(64, Fft::cd(1.0, 0.0));
    Fft::compute(data);

    CHECK(std::abs(data[0]) == Catch::Approx(64.0).margin(1e-9));
    for (size_t i = 1; i < data.size(); i++)
        CHECK(std::abs(data[i]) == Catch::Approx(0.0).margin(1e-9));
}

TEST_CASE("FFT single frequency sine peaks at correct bin", "[fft]") {
    constexpr int N = 256;
    constexpr int targetBin = 10;

    std::vector<Fft::cd> data(N);
    for (int i = 0; i < N; i++)
        data[i] = Fft::cd(std::sin(2.0 * PI * targetBin * i / N), 0.0);

    Fft::compute(data);

    // Find bin with maximum magnitude
    int maxBin = 0;
    double maxMag = 0.0;
    for (int i = 0; i < N / 2; i++) {
        double mag = std::abs(data[i]);
        if (mag > maxMag) {
            maxMag = mag;
            maxBin = i;
        }
    }
    CHECK(maxBin == targetBin);
}

TEST_CASE("FFT round-trip recovers original signal", "[fft]") {
    constexpr int N = 128;
    std::vector<Fft::cd> original(N);
    for (int i = 0; i < N; i++)
        original[i] = Fft::cd(std::sin(2.0 * PI * 7 * i / N) + 0.5 * std::cos(2.0 * PI * 23 * i / N), 0.0);

    auto data = original;
    Fft::compute(data, false);
    Fft::compute(data, true);

    for (int i = 0; i < N; i++) {
        CHECK(data[i].real() == Catch::Approx(original[i].real()).margin(1e-9));
        CHECK(data[i].imag() == Catch::Approx(original[i].imag()).margin(1e-9));
    }
}

TEST_CASE("FFT edge sizes", "[fft]") {
    SECTION("size 1 is identity") {
        std::vector<Fft::cd> data = {Fft::cd(42.0, 0.0)};
        Fft::compute(data);
        CHECK(data[0].real() == Catch::Approx(42.0));
    }

    SECTION("size 2") {
        std::vector<Fft::cd> data = {Fft::cd(1.0, 0.0), Fft::cd(1.0, 0.0)};
        Fft::compute(data);
        CHECK(std::abs(data[0]) == Catch::Approx(2.0).margin(1e-9));
        CHECK(std::abs(data[1]) == Catch::Approx(0.0).margin(1e-9));
    }
}
