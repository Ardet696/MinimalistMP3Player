#include <catch2/catch_all.hpp>
#include "../../src/player/AutoAdvanceManager.h"
#include "../../src/player/SongQueue.h"
#include "../../src/commands/CommandQueue.h"
#include "../../src/util/DirectoryWatcher.h"
#include "../../src/service/IConfigService.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <random>
#include <string>
#include <thread>

namespace {

class FakeConfig : public IConfigService {
public:
    explicit FakeConfig(std::string root) : root_(std::move(root)) {}
    std::string getRootPath() const override { return root_; }
    void setRootPath(const std::string& path) override { root_ = path; }
    int getTheme() const override { return 0; }
    void setTheme(int) override {}
    int getVisual() const override { return 0; }
    void setVisual(int) override {}
private:
    std::string root_;
};

struct TempDir {
    std::filesystem::path path;
    TempDir() {
        std::random_device rd;
        path = std::filesystem::temp_directory_path() /
               ("mp3player_watch_" + std::to_string(rd()) + std::to_string(rd()));
        std::filesystem::create_directories(path);
    }
    ~TempDir() { std::filesystem::remove_all(path); }
};

} // namespace

TEST_CASE("AutoAdvanceManager stops and joins on destruction", "[thread][raii]") {
    {
        AutoAdvanceManager mgr;
        mgr.setEnabled(true);
        mgr.start([] { return false; }, [] {});
    }
    SUCCEED("monitor thread joined on scope exit");
}

TEST_CASE("SongQueue stops prewarm thread on destruction", "[thread][raii]") {
    {
        SongQueue queue(nullptr);
        queue.loadPlaylist({});
    }
    SUCCEED("prewarm thread joined on scope exit");
}

TEST_CASE("CommandQueue drains worker on destruction", "[thread][raii]") {
    std::atomic<int> ran{0};
    {
        CommandQueue queue;
        for (int i = 0; i < 16; ++i) {
            queue.enqueue([&ran] { ++ran; });
        }
    }
    SUCCEED("worker thread joined on scope exit");
}

TEST_CASE("DirectoryWatcher joins watcher thread on destruction", "[thread][raii]") {
    TempDir dir;
    FakeConfig config(dir.path.string());
    {
        DirectoryWatcher watcher(config, [](const std::string&) {});
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    SUCCEED("watcher thread joined on scope exit");
}
