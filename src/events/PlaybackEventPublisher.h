#ifndef MP3PLAYER_PLAYBACKEVENTPUBLISHER_H
#define MP3PLAYER_PLAYBACKEVENTPUBLISHER_H

#include "PlaybackEvent.h"
#include <functional>
#include <vector>
#include <mutex>

/**
 *  Observer pattern for playback events.
 *
 * Allows UI and other components to subscribe to playback state changes
 * without tight coupling.
 */
class PlaybackEventPublisher {
public:
    using EventCallback = std::function<void(const PlaybackEvent&)>;

    PlaybackEventPublisher() = default;

    PlaybackEventPublisher(const PlaybackEventPublisher&) = delete;
    PlaybackEventPublisher& operator=(const PlaybackEventPublisher&) = delete;

    int subscribe(EventCallback callback);
    void unsubscribe(int subscriptionId);
    void publish(const PlaybackEvent& event);

private:
    struct Subscription {
        int id;
        EventCallback callback;
    };

    std::vector<Subscription> subscribers_;
    std::mutex mutex_;
    int nextId_{0};
};

#endif
