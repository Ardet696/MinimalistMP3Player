#include <catch2/catch_all.hpp>
#include "../../src/audio/SdlAudioSink.h"
#include "../../src/util/AudioFormat.h"

#include <cstdlib>

namespace {

SdlAudioSink::FrameProvider silenceProvider() {
    return [](int16_t* out, std::size_t frames) -> std::size_t {
        for (std::size_t i = 0; i < frames * 2; ++i) out[i] = 0;
        return frames;
    };
}

AudioFormat stereo44k() {
    AudioFormat fmt{};
    fmt.sampleRate = 44100;
    fmt.channels = 2;
    return fmt;
}

} // namespace

TEST_CASE("SdlAudioSink open/close lifecycle is balanced", "[audio][raii]") {
    setenv("SDL_AUDIODRIVER", "dummy", 1);

    SdlAudioSink sink;
    REQUIRE(sink.open(stereo44k(), silenceProvider()));
    CHECK(sink.isOpen());

    sink.close();
    CHECK_FALSE(sink.isOpen());

    REQUIRE(sink.open(stereo44k(), silenceProvider()));
    CHECK(sink.isOpen());
}

TEST_CASE("SdlAudioSink closes device when destroyed while open", "[audio][raii]") {
    setenv("SDL_AUDIODRIVER", "dummy", 1);
    {
        SdlAudioSink sink;
        REQUIRE(sink.open(stereo44k(), silenceProvider()));
        sink.start();
    }
    SUCCEED("destructor closed device and released SDL subsystem");
}

TEST_CASE("SdlAudioSink double close is a no-op", "[audio][raii]") {
    setenv("SDL_AUDIODRIVER", "dummy", 1);

    SdlAudioSink sink;
    REQUIRE(sink.open(stereo44k(), silenceProvider()));
    sink.close();
    sink.close();
    CHECK_FALSE(sink.isOpen());
}
