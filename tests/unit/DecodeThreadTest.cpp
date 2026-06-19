#include <catch2/catch_all.hpp>
#include "../../src/player/DecodeThread.h"
#include "../../src/decode/IAudioDecoder.h"
#include "../../src/util/RingBuffer.h"

#include <algorithm>
#include <atomic>
#include <cstdint>

namespace {

class FakeDecoder : public IAudioDecoder {
public:
    explicit FakeDecoder(std::uint64_t totalFrames) : remaining_(totalFrames) {}

    bool open(const std::filesystem::path&) override { open_ = true; return true; }
    void close() override { open_ = false; }
    bool isOpen() const override { return open_; }
    int sampleRate() const override { return 44100; }
    int channels() const override { return 2; }
    std::uint64_t totalSamples() const override { return remaining_.load(); }

    std::size_t decodeFrames(std::span<int16_t> out, std::size_t outFrames) override {
        const std::size_t want = std::min<std::size_t>(outFrames, remaining_.load());
        const std::size_t samples = std::min(out.size(), want * 2);
        std::fill_n(out.begin(), samples, int16_t{0});
        const std::size_t frames = samples / 2;
        remaining_ -= frames;
        return frames;
    }

    const std::string& lastError() const override { return err_; }

private:
    std::atomic<std::uint64_t> remaining_;
    bool open_ = false;
    std::string err_;
};

} // namespace

TEST_CASE("DecodeThread joins on destruction without explicit stop", "[thread][raii]") {
    FakeDecoder decoder(1'000'000);
    RingBuffer<int16_t> ring(8192);

    {
        DecodeThread thread;
        thread.start(&decoder, &ring);
        CHECK(thread.isRunning());
    }
    SUCCEED("destructor stopped and joined the decode thread");
}

TEST_CASE("DecodeThread reaches end of stream and joins cleanly", "[thread][raii]") {
    FakeDecoder decoder(2048);
    RingBuffer<int16_t> ring(1 << 16);

    DecodeThread thread;
    thread.start(&decoder, &ring, 256);

    for (int i = 0; i < 1000 && !thread.isEndOfStream(); ++i) {
        int16_t scratch[512];
        ring.read(scratch, 512);
    }

    thread.stop();
    CHECK_FALSE(thread.isRunning());
}
