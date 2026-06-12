#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE  // expose setenv() under -std=c++20 on macOS
#endif

#include <catch2/catch_all.hpp>
#include "../../src/service/ConfigService.h"
#include <filesystem>
#include <cstdlib>
#include <random>
#include <string>

// Helper: create a temp dir and redirect HOME so ConfigService writes there
struct TempHomeGuard {
    std::filesystem::path tmpDir;
    std::string oldHome;

    TempHomeGuard() {
        std::random_device rd;
        tmpDir = std::filesystem::temp_directory_path() /
                 ("mp3player_test_" + std::to_string(rd()) + std::to_string(rd()));
        std::filesystem::create_directories(tmpDir);

        const char* h = std::getenv("HOME");
        oldHome = h ? h : "";
        setenv("HOME", tmpDir.c_str(), 1);
    }

    ~TempHomeGuard() {
        setenv("HOME", oldHome.c_str(), 1);
        std::filesystem::remove_all(tmpDir);
    }
};

TEST_CASE("ConfigService defaults on fresh config", "[config]") {
    TempHomeGuard guard;
    ConfigService config;

    CHECK(config.getRootPath().empty());
    CHECK(config.getTheme() == 0);
    CHECK(config.getVisual() == 0);
}

TEST_CASE("ConfigService set and get round-trip", "[config]") {
    TempHomeGuard guard;
    ConfigService config;

    config.setRootPath("/home/user/Music");
    CHECK(config.getRootPath() == "/home/user/Music");

    config.setTheme(2);
    CHECK(config.getTheme() == 2);

    config.setVisual(3);
    CHECK(config.getVisual() == 3);
}

TEST_CASE("ConfigService overwrites preserve other keys", "[config]") {
    TempHomeGuard guard;
    ConfigService config;

    config.setRootPath("/music");
    config.setTheme(1);
    config.setVisual(4);

    // Overwrite one key
    config.setTheme(3);

    // Other keys should survive
    CHECK(config.getRootPath() == "/music");
    CHECK(config.getTheme() == 3);
    CHECK(config.getVisual() == 4);
}

TEST_CASE("ConfigService persists across instances", "[config]") {
    TempHomeGuard guard;

    {
        ConfigService config;
        config.setRootPath("/persisted");
        config.setTheme(2);
    }

    {
        ConfigService config;
        CHECK(config.getRootPath() == "/persisted");
        CHECK(config.getTheme() == 2);
    }
}
