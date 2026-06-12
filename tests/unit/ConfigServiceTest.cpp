#include <catch2/catch_all.hpp>
#include "../../src/service/ConfigService.h"
#include <filesystem>
#include <cstdlib>

// Helper: create a temp dir and redirect HOME so ConfigService writes there
struct TempHomeGuard {
    std::filesystem::path tmpDir;
    std::string oldHome;

    TempHomeGuard() {
        tmpDir = std::filesystem::temp_directory_path() / "mp3player_test_XXXXXX";
        tmpDir = std::filesystem::path(mkdtemp(tmpDir.string().data()));

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
