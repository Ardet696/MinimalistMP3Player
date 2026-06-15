#pragma once
#include <string>
#include <vector>
#include <mutex>

enum class NotifyLevel
{
    Info,
    Error
};

struct Notification {
    std::string message;
    NotifyLevel level;
};

class NotificationBus {
public:
    void push(std::string message, NotifyLevel level = NotifyLevel::Info) {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_.push_back({std::move(message), level});
    }

    std::vector<Notification> drain() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Notification> out;
        out.swap(pending_);
        return out;
    }

private:
    std::vector<Notification> pending_;
    std::mutex mutex_;
};
