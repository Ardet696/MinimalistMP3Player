#include "AutoAdvanceManager.h"
#include "../config/Config.h"
#include <chrono>

AutoAdvanceManager::AutoAdvanceManager()
    : enabled_(true)
{
}

AutoAdvanceManager::~AutoAdvanceManager() {
    stop();
}

void AutoAdvanceManager::start(std::function<bool()> shouldAdvance, AdvanceCallback onAdvance) {
    stop();  // Stop any existing monitor

    shouldAdvance_ = std::move(shouldAdvance);
    onAdvance_ = std::move(onAdvance);


    monitorThread_ = std::jthread([this](std::stop_token stopToken) {// Start monitor thread
        monitorLoop(stopToken);
    });
}

void AutoAdvanceManager::stop() {
    if (monitorThread_.joinable()) {
        monitorThread_.request_stop();
        monitorThread_ = std::jthread();  // Reset and wait for join
    }

    shouldAdvance_ = nullptr;
    onAdvance_ = nullptr;
}

void AutoAdvanceManager::setEnabled(bool enabled) {
    enabled_.store(enabled, std::memory_order_release);
}

bool AutoAdvanceManager::isEnabled() const {
    return enabled_.load(std::memory_order_acquire);
}

void AutoAdvanceManager::monitorLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        // Check if we should advance
        if (enabled_.load(std::memory_order_acquire) && shouldAdvance_ && shouldAdvance_()) {
            // Trigger advance callback
            if (onAdvance_) {
                onAdvance_();
            }
        }

        // Sleep before next check
        std::this_thread::sleep_for(Config::AUTO_ADVANCE_POLL_INTERVAL_MS);
    }
}