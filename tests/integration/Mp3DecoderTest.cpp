#include <catch2/catch_all.hpp>
#include "../../src/decode/Mp3Decoder.h"
#include <vector>
#include <filesystem>

static std::filesystem::path fixtureDir() {
    return std::filesystem::path(FIXTURE_DIR);
}

TEST_CASE("Mp3Decoder opens valid MP3", "[decoder]") {
    Mp3Decoder dec;
    bool ok = dec.open(fixtureDir() / "sine_440hz_1s.mp3");
    REQUIRE(ok);
    CHECK(dec.isOpen());
    CHECK(dec.sampleRate() == 44100);
    CHECK(dec.channels() >= 1);
    CHECK(dec.totalSamples() > 0);
}

TEST_CASE("Mp3Decoder decodes frames", "[decoder]") {
    Mp3Decoder dec;
    REQUIRE(dec.open(fixtureDir() / "sine_440hz_1s.mp3"));

    std::vector<int16_t> buf(1152 * dec.channels());
    size_t totalFrames = 0;
    size_t frames;

    do {
        frames = dec.decodeFrames(buf, 1152);
        totalFrames += frames;
    } while (frames > 0);

    // 1 second at 44100 Hz should give roughly 44100 frames (MP3 padding may vary)
    CHECK(totalFrames > 40000);
    CHECK(totalFrames < 50000);
}

TEST_CASE("Mp3Decoder silence file decodes to near-zero", "[decoder]") {
    Mp3Decoder dec;
    REQUIRE(dec.open(fixtureDir() / "silence_1s.mp3"));

    std::vector<int16_t> buf(1152 * dec.channels());
    int64_t sumAbs = 0;
    size_t totalSamples = 0;
    size_t frames;

    do {
        frames = dec.decodeFrames(buf, 1152);
        for (size_t i = 0; i < frames * dec.channels(); i++)
            sumAbs += std::abs(buf[i]);
        totalSamples += frames * dec.channels();
    } while (frames > 0);

    // Average sample should be near zero for silence
    double avgAbs = static_cast<double>(sumAbs) / std::max<size_t>(totalSamples, 1);
    CHECK(avgAbs < 100.0); // Very generous threshold for MP3 codec noise
}

TEST_CASE("Mp3Decoder fails gracefully on nonexistent file", "[decoder]") {
    Mp3Decoder dec;
    bool ok = dec.open("/nonexistent/file.mp3");
    CHECK_FALSE(ok);
    CHECK_FALSE(dec.isOpen());
}

TEST_CASE("Mp3Decoder close and reopen", "[decoder]") {
    Mp3Decoder dec;
    REQUIRE(dec.open(fixtureDir() / "silence_1s.mp3"));
    CHECK(dec.isOpen());

    dec.close();
    CHECK_FALSE(dec.isOpen());

    REQUIRE(dec.open(fixtureDir() / "sine_440hz_1s.mp3"));
    CHECK(dec.isOpen());
    CHECK(dec.sampleRate() == 44100);
}
