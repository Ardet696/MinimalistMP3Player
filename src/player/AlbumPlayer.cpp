#include "AlbumPlayer.h"
#include <iostream>
#include <thread>
#include <chrono>
AlbumPlayer::AlbumPlayer()
    : autoAdvanceEnabled_(true)
{
}

AlbumPlayer::~AlbumPlayer() {
    // jthread automatically requests stop and joins on destruction
    stop();
}

void AlbumPlayer::loadAlbum(const std::vector<std::filesystem::path>& songPaths) {
    stop();  // Stop any current playback

    queue_.loadPlaylist(songPaths);

    if (!songPaths.empty()) {
        std::cerr << "Album loaded: " << songPaths.size() << " tracks\n";
        startAutoAdvanceThread();
    }
}

void AlbumPlayer::play() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.getPlaylistSize() == 0) {
        std::cerr << "No album loaded\n";
        return;
    }
    if (engine_.isPaused()) {
        engine_.play();
        return;
    }

    // If stopped, load current song from queue
    const std::filesystem::path currentSong = queue_.getCurrentSongPath();
    if (currentSong.empty()) {
        std::cerr << "No current song\n";
        return;
    }

    loadAndPlaySong(currentSong);
}

void AlbumPlayer::pause() {
    std::lock_guard<std::mutex> lock(mutex_);
    engine_.pause();
}

void AlbumPlayer::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    engine_.stop();
    queue_.clear();
    // jthread will automatically stop and join when we exit scope or on destruction
}

void AlbumPlayer::next() {
    std::lock_guard<std::mutex> lock(mutex_);

    const std::filesystem::path nextSong = queue_.next();
    if (nextSong.empty()) {
        std::cerr << "End of album reached\n";
        engine_.stop();
        queue_.clear();
        return;
    }

    loadAndPlaySong(nextSong);
}

void AlbumPlayer::previous() {
    std::lock_guard<std::mutex> lock(mutex_);

    const std::filesystem::path prevSong = queue_.previous();
    if (prevSong.empty()) {
        std::cerr << "Already at first song\n";
        return;
    }

    loadAndPlaySong(prevSong);
}

bool AlbumPlayer::isPlaying() const {
    return engine_.isPlaying();
}

bool AlbumPlayer::isPaused() const {
    return engine_.isPaused();
}

bool AlbumPlayer::isStopped() const {
    return engine_.isStopped();
}

std::filesystem::path AlbumPlayer::getCurrentSong() const {
    return queue_.getCurrentSongPath();
}

int AlbumPlayer::getCurrentTrackNumber() const {
    return queue_.getCurrentIndex() + 1;  // 1-indexed for user display
}

int AlbumPlayer::getTotalTracks() const {
    return queue_.getPlaylistSize();
}

bool AlbumPlayer::loadAndPlaySong(const std::filesystem::path& songPath) {
    // IMPORTANT: This function must be called with mutex_ held!

    engine_.stop();

    if (!engine_.load(songPath)) {
        std::cerr << "Failed to load: " << songPath << "\n";
        return false;
    }

    std::cerr << "\nNow playing: " << songPath.filename().string() << "\n";
    std::cerr << "Track " << getCurrentTrackNumber() << " of " << getTotalTracks() << "\n";

    engine_.play();
    return true;
}

void AlbumPlayer::checkAutoAdvance() {
    if (!autoAdvanceEnabled_) {
        return;
    }

    // Check for end-of-stream while still playing
    // Use try_lock to avoid blocking the auto-advance thread
    std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Mutex is busy, skip this check
        return;
    }

    if (engine_.isPlaying() && engine_.hasReachedEndOfStream()) {
        std::cerr << "\nSong finished, auto-advancing...\n";

        const std::filesystem::path nextSong = queue_.next();
        if (nextSong.empty()) {
            std::cerr << "End of album reached\n";
            engine_.stop();
            queue_.clear();
            return;
        }

        loadAndPlaySong(nextSong);
    }
}

void AlbumPlayer::startAutoAdvanceThread() {
    // Stop any existing thread by requesting stop (jthread will join automatically)
    if (autoAdvanceThread_.joinable()) {
        autoAdvanceThread_.request_stop();
        autoAdvanceThread_ = std::jthread();  // Reset to empty jthread
    }

    // Start new thread with stop_token support
    // Use lambda to properly bind member function with stop_token
    autoAdvanceThread_ = std::jthread([this](std::stop_token stopToken) {
        autoAdvanceLoop(stopToken);
    });
}

void AlbumPlayer::autoAdvanceLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        checkAutoAdvance();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
