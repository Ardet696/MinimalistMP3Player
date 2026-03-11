#ifndef MP3PLAYER_ALBUMPLAYER_H
#define MP3PLAYER_ALBUMPLAYER_H

#include <filesystem>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include "PlaybackEngine.h"
#include "SongQueue.h"

/**
 * AlbumPlayer - High-level album playback controller.
 *
 * Combines PlaybackEngine + SongQueue for seamless album playback:
 *   - Handles song transitions automatically
 *   - Pre-warms upcoming songs in background
 *   - Supports skip next/previous song
 *   - Detects end of a song and moves on to next track
 */
class AlbumPlayer {
public:
    AlbumPlayer();
    ~AlbumPlayer();

    void loadAlbum(const std::vector<std::filesystem::path>& songPaths);


    void play();
    void pause();
    void stop();
    void next();
    void previous();
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    std::filesystem::path getCurrentSong() const;
    int getCurrentTrackNumber() const;
    int getTotalTracks() const;

    /**
     * Check if current song has ended and auto-advance if needed.
     * Should be called periodically by UI.
     */
    void checkAutoAdvance();

private:

    bool loadAndPlaySong(const std::filesystem::path& songPath);
    void startAutoAdvanceThread();
    void autoAdvanceLoop(std::stop_token stopToken);

    PlaybackEngine engine_;
    SongQueue queue_;
    bool autoAdvanceEnabled_;

    std::jthread autoAdvanceThread_;
    mutable std::mutex mutex_;  // Protects engine operations from concurrent access
};

#endif
