#include "DirectoryWatcher.h"
#include "../service/IConfigService.h"
#include <chrono>
#include <filesystem>

DirectoryWatcher::DirectoryWatcher(IConfigService& config, ChangeCallback onChange)
    : config_(config), onChange_(std::move(onChange)),
      thread_([this](std::stop_token stop) { run(stop); })
{}

std::set<std::string> DirectoryWatcher::snapshot(const std::string& root) {
    std::set<std::string> entries;
    if (root.empty()) return entries;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(root, ec))
        if (entry.is_directory(ec))
            entries.insert(entry.path().filename().string());
    return entries;
}

void DirectoryWatcher::run(std::stop_token stop) {
    auto last = snapshot(config_.getRootPath());
    while (!stop.stop_requested()) {
        for (int i = 0; i < 20 && !stop.stop_requested(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (stop.stop_requested()) break;

        std::string root = config_.getRootPath();
        if (root.empty()) continue;

        auto current = snapshot(root);
        if (current != last) {
            last = current;
            onChange_(root);
        }
    }
}
