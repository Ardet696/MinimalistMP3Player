#include "ScreenRefreshTimer.h"
#include <ftxui/component/event.hpp>

ScreenRefreshTimer::ScreenRefreshTimer(ftxui::App& screen, std::chrono::milliseconds interval)
    : thread_([&screen, interval](std::stop_token stop) {
        while (!stop.stop_requested()) {
            std::this_thread::sleep_for(interval);
            screen.PostEvent(ftxui::Event::Custom);
        }
    })
{}
