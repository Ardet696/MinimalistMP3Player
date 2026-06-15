#pragma once
#include <functional>
#include <set>
#include <string>
#include <thread>

class IConfigService;

class DirectoryWatcher {
public:
    using ChangeCallback = std::function<void(const std::string& path)>;

    DirectoryWatcher(IConfigService& config, ChangeCallback onChange);

private:
    static std::set<std::string> snapshot(const std::string& root);
    void run(std::stop_token stop);

    IConfigService& config_;
    ChangeCallback onChange_;
    std::jthread thread_;
};
