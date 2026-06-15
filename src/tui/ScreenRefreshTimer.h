#pragma once
#include <chrono>
#include <thread>
#include <ftxui/component/app.hpp>

class ScreenRefreshTimer {
public:
    ScreenRefreshTimer(ftxui::App& screen, std::chrono::milliseconds interval);

private:
    std::jthread thread_;
};
