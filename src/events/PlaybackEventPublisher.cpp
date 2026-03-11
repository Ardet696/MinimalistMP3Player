#include "PlaybackEventPublisher.h"
#include <algorithm>

int PlaybackEventPublisher::subscribe(EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    const int id = nextId_++;
    subscribers_.push_back({id, std::move(callback)});
    return id;
}

void PlaybackEventPublisher::unsubscribe(int subscriptionId) {
    std::lock_guard<std::mutex> lock(mutex_);

    subscribers_.erase(
        std::remove_if(subscribers_.begin(), subscribers_.end(),
                       [subscriptionId](const Subscription& sub) {
                           return sub.id == subscriptionId;
                       }),
        subscribers_.end()
    );
}

void PlaybackEventPublisher::publish(const PlaybackEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Call all subscriber callbacks
    for (const auto& sub : subscribers_) {
        if (sub.callback) {
            sub.callback(event);
        }
    }
}
