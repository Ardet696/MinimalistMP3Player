#include "PlaybackController.h"
#include <iostream>

PlaybackController::PlaybackController() {
}

PlaybackController::~PlaybackController() {
    stop();
}

void PlaybackController::loadAlbum(const std::vector<std::filesystem::path>& songPaths) {
    stop();

    {
        std::lock_guard lock(mutex_);
        currentAlbum_ = songPaths;
    }
    queue_.loadPlaylist(songPaths);

    if (!songPaths.empty()) {
        std::cerr << "Album loaded: " << songPaths.size() << " tracks\n";

        // Start auto-advance monitoring
        autoAdvance_.start(
            [this]() { return shouldAutoAdvance(); },
            [this]() { handleAutoAdvance(); }
        );

        eventPublisher_.publish(PlaybackEvent(PlaybackEventType::TrackChanged));
    }
}

void PlaybackController::play() {
    std::lock_guard lock(mutex_);

    if (queue_.getPlaylistSize() == 0) {
        std::cerr << "No album loaded\n";
        return;
    }

    if (engine_.isPaused()) {
        engine_.play();
        eventPublisher_.publish(PlaybackEvent(PlaybackEventType::SongResumed));
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

void PlaybackController::pause() {
    std::lock_guard lock(mutex_);
    engine_.pause();
    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::SongPaused));
}

void PlaybackController::stop() {
    std::lock_guard lock(mutex_);
    engine_.stop();
    queue_.clear();
    autoAdvance_.stop();
    currentAlbum_.clear();
    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::SongStopped));
}

void PlaybackController::next() {
    std::lock_guard lock(mutex_);

    const std::filesystem::path nextSong = queue_.next();
    if (nextSong.empty()) {
        std::cerr << "End of album reached\n";
        engine_.stop();
        queue_.clear();
        eventPublisher_.publish(PlaybackEvent(PlaybackEventType::AlbumFinished));
        return;
    }

    loadAndPlaySong(nextSong);
    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::TrackChanged,nextSong,getCurrentTrackNumber(),getTotalTracks()));
}

void PlaybackController::previous() {
    std::lock_guard lock(mutex_);

    const std::filesystem::path prevSong = queue_.previous();
    if (prevSong.empty()) {
        std::cerr << "Already at first song\n";
        return;
    }

    loadAndPlaySong(prevSong);
    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::TrackChanged,prevSong,getCurrentTrackNumber(),  getTotalTracks()));
}

bool PlaybackController::isPlaying() const {
    return engine_.isPlaying();
}

bool PlaybackController::isPaused() const {
    return engine_.isPaused();
}

bool PlaybackController::isStopped() const {
    return engine_.isStopped();
}

std::filesystem::path PlaybackController::getCurrentSong() const {
    return queue_.getCurrentSongPath();
}

int PlaybackController::getCurrentTrackNumber() const {
    return queue_.getCurrentIndex() + 1;  // 1-indexed for user display
}

int PlaybackController::getTotalTracks() const {
    return queue_.getPlaylistSize();
}

std::vector<float> PlaybackController::getSpectrumBars() const
{
    return engine_.getSpectrumAnalyzer().getBars();
}

std::vector<float> PlaybackController::getSpectrumMagnitudes() const
{
    return engine_.getSpectrumAnalyzer().getSpectrumMagnitudes();
}

float PlaybackController::getPlaybackProgress() const {
    return engine_.getPlaybackProgress();
}

float PlaybackController::getBpmLoadFactor() const {
    return engine_.getBpmLoadFactor();
}

std::string PlaybackController::getCurrentAlbum() const {
    return currentAlbumName_;
}

void PlaybackController::setCurrentAlbum(const std::string& albumName) {
    currentAlbumName_ = albumName;
}

PlaybackEventPublisher& PlaybackController::getEventPublisher() {
    return eventPublisher_;
}

void PlaybackController::setAutoAdvanceEnabled(bool enabled) {
    autoAdvance_.setEnabled(enabled);
}

bool PlaybackController::isAutoAdvanceEnabled() const {
    return autoAdvance_.isEnabled();
}

bool PlaybackController::loadAndPlaySong(const std::filesystem::path& songPath) {
    // This function has to  be called with mutex_ held!

    engine_.stop();

    if (!engine_.load(songPath)) {
        std::cerr << "Failed to load: " << songPath << "\n";
        eventPublisher_.publish(PlaybackEvent(PlaybackEventType::LoadFailed, songPath, 0, 0, "Failed to load song"));
        return false;
    }

    std::cerr << "\nNow playing: " << songPath.filename().string() << "\n";
    std::cerr << "Track " << getCurrentTrackNumber() << " of " << getTotalTracks() << "\n";

    engine_.play();

    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::SongStarted,songPath,getCurrentTrackNumber(), getTotalTracks()));
    return true;
}

void PlaybackController::playSongAtIndex(int index) {
    std::lock_guard lock(mutex_);

    if (currentAlbum_.empty()) {
        std::cerr << "No album loaded\n";
        return;
    }

    if (index < 0 || index >= static_cast<int>(currentAlbum_.size())) {
        std::cerr << "Invalid index: " << index << "\n";
        return;
    }

    queue_.loadPlaylist(currentAlbum_);
    queue_.skipTo(index);

    auto songPath = queue_.getCurrentSongPath();
    if (songPath.empty()) {
        std::cerr << "No song at index: " << index << "\n";
        return;
    }

    loadAndPlaySong(songPath);

    autoAdvance_.start(
        [this]() { return shouldAutoAdvance(); },
        [this]() { handleAutoAdvance(); }
    );
}

void PlaybackController::handleAutoAdvance() {
    // Use try_lock to avoid blocking the auto-advance thread
    std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
    if (!lock.owns_lock()) {
        // Mutex is busy, skip this iteration
        return;
    }

    std::cerr << "\nSong finished, auto-advancing...\n";

    const std::filesystem::path nextSong = queue_.next();
    if (nextSong.empty()) {
        std::cerr << "End of album reached\n";
        engine_.stop();
        queue_.clear();
        eventPublisher_.publish(PlaybackEvent(PlaybackEventType::AlbumFinished));
        return;
    }

    loadAndPlaySong(nextSong);
    eventPublisher_.publish(PlaybackEvent(PlaybackEventType::SongFinished));
}

void PlaybackController::setVolume(int percent) {
    engine_.setVolume(percent);
}

int PlaybackController::getVolume() const {
    return engine_.getVolume();
}

void PlaybackController::setOutputDevice(const std::string& deviceName) {
    std::lock_guard lock(mutex_);
    engine_.setOutputDevice(deviceName);

    // If currently playing, reload the current song through the new device
    const auto currentSong = queue_.getCurrentSongPath();
    if (!currentSong.empty() && !engine_.isStopped()) {
        engine_.stop();
        if (engine_.load(currentSong)) {
            engine_.play();
        }
    }
}

std::string PlaybackController::getOutputDevice() const {
    return engine_.getOutputDevice();
}

std::vector<std::string> PlaybackController::listOutputDevices() {
    return PlaybackEngine::listOutputDevices();
}

bool PlaybackController::shouldAutoAdvance() const {
    return engine_.isPlaying() && engine_.hasReachedEndOfStream();
}