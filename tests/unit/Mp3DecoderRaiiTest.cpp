#include <catch2/catch_all.hpp>
#include "../../src/decode/Mp3Decoder.h"
#include <filesystem>
#include <utility>

static std::filesystem::path fixtureDir() {
    return std::filesystem::path(FIXTURE_DIR);
}

TEST_CASE("Mp3Decoder releases handle when destroyed while open", "[decoder][raii]") {
    {
        Mp3Decoder dec;
        REQUIRE(dec.open(fixtureDir() / "sine_440hz_1s.mp3"));
        REQUIRE(dec.isOpen());
    }
    SUCCEED("no leak reported by sanitizer on scope exit without close()");
}

TEST_CASE("Mp3Decoder move transfers ownership without leak", "[decoder][raii]") {
    Mp3Decoder src;
    REQUIRE(src.open(fixtureDir() / "silence_1s.mp3"));
    REQUIRE(src.isOpen());

    Mp3Decoder dst = std::move(src);
    CHECK(dst.isOpen());
}
