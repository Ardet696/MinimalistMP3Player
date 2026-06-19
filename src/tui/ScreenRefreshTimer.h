#pragma once
#include <chrono>
#include <thread>
#include <ftxui/component/app.hpp>

class ScreenRefreshTimer {
public:
    ScreenRefreshTimer(ftxui::App& screen, std::chrono::milliseconds interval);

    ScreenRefreshTimer(const ScreenRefreshTimer&) = delete;
    ScreenRefreshTimer& operator=(const ScreenRefreshTimer&) = delete;
    ScreenRefreshTimer(ScreenRefreshTimer&&) = delete;
    ScreenRefreshTimer& operator=(ScreenRefreshTimer&&) = delete;

private:
    std::jthread thread_;
};
