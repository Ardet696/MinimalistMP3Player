#ifndef MP3PLAYER_AUTOADVANCEMANAGER_H
#define MP3PLAYER_AUTOADVANCEMANAGER_H

#include <functional>
#include <atomic>
#include <thread>
/**
 * AutoAdvanceManager - Monitors playback and triggers auto-advance.
 *
 * Runs a background thread that periodically checks if the current song
 * has finished and triggers the advance callback.
 *
 * Separated from PlaybackController for single responsibility principle.
 */
class AutoAdvanceManager {
public:
    using AdvanceCallback = std::function<void()>;

    AutoAdvanceManager();
    ~AutoAdvanceManager();

    // Non copyable, non-movable
    AutoAdvanceManager(const AutoAdvanceManager&) = delete;
    AutoAdvanceManager& operator=(const AutoAdvanceManager&) = delete;

    /**
     * Start monitoring for auto-advance.
     * @param shouldAdvance Function that returns true when advance should occur
     * @param onAdvance Callback to execute when advance is triggered
     */
    void start(std::function<bool()> shouldAdvance, AdvanceCallback onAdvance);
    void stop();

    /**
     * Enable/disable auto-advance.
     */
    void setEnabled(bool enabled);

    /**
     * Check if auto-advance is enabled.
     */
    bool isEnabled() const;

private:
    void monitorLoop(std::stop_token stopToken);

    std::function<bool()> shouldAdvance_;
    AdvanceCallback onAdvance_;

    std::atomic<bool> enabled_;
    std::jthread monitorThread_;
};

#endif