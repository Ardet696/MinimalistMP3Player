#include "LibraryService.h"

LibraryService::LibraryService(MusicLibrary& library, PlaybackController& controller): library_(library), controller_(controller) {}

std::vector<std::string> LibraryService::getAlbumNames() const {
    std::lock_guard lock(mutex_);
    std::vector<std::string> names;
    for (const auto& album : library_.getAlbums()) {
        names.push_back(album.getTitle());
    }
    return names;
}

std::vector<std::string> LibraryService::getSongNames(const std::string& albumName) const {
    std::lock_guard lock(mutex_);
    const auto* album = library_.findAlbumByTitle(albumName);
    if (!album) return {};

    std::vector<std::string> names;
    for (const auto& song : album->getSongs()) {
        names.push_back(song.getTitle());
    }
    return names;
}

std::vector<std::vector<std::string>> LibraryService::getAllSongNames() const {
    std::lock_guard lock(mutex_);
    std::vector<std::vector<std::string>> all;
    for (const auto& album : library_.getAlbums()) {
        std::vector<std::string> names;
        for (const auto& song : album.getSongs()) {
            names.push_back(song.getTitle());
        }
        all.push_back(std::move(names));
    }
    return all;
}

bool LibraryService::setRootPath(const std::string& path, std::string& outError) {
    MusicLibrary newLibrary = scanner_.scanFromUserInput(path, &outError);
    if (!outError.empty()) {
        return false;
    }
    std::lock_guard lock(mutex_);
    library_ = std::move(newLibrary);
    return true;
}

void LibraryService::playSong(const std::string& albumName, int songIndex) {
    std::lock_guard lock(mutex_);
    const auto* album = library_.findAlbumByTitle(albumName);
    if (!album) return;

    const auto& songs = album->getSongs();
    if (songIndex < 0 || songIndex >= static_cast<int>(songs.size())) return;

    controller_.setCurrentAlbum(albumName);

    std::vector<std::filesystem::path> songPaths;
    songPaths.reserve(songs.size());
    for (const auto& song : songs) {
        songPaths.push_back(song.getFilePath());
    }

    controller_.loadAlbum(songPaths);
    controller_.playSongAtIndex(songIndex);
}

void LibraryService::prevSong() {
    controller_.previous();
}

void LibraryService::nextSong() {
    controller_.next();
}

void LibraryService::resumePlayback() {
    controller_.play();
}

void LibraryService::stopPlayback() {
    controller_.pause();
}

std::vector<float> LibraryService::getSpectrumBars() const {
    return controller_.getSpectrumBars();
}
std::vector<float> LibraryService::getSpectrumMagnitudes() const
{   return controller_.getSpectrumMagnitudes();
}

float LibraryService::getPlaybackProgress() const {
    return controller_.getPlaybackProgress();
}

float LibraryService::getBpmLoadFactor() const {
    return controller_.getBpmLoadFactor();
}

std::string LibraryService::getCurrentAlbum() const {
    return controller_.getCurrentAlbum();
}

int LibraryService::getCurrentTrackIndex() const {
    return controller_.getCurrentTrackNumber() - 1;
}

std::string LibraryService::getCurrentSongName() const {
    auto path = controller_.getCurrentSong();
    if (path.empty()) return "";
    return path.stem().string();
}

std::string LibraryService::getNextSongName() const {
    int current = controller_.getCurrentTrackNumber(); // 1-based
    int total = controller_.getTotalTracks();
    if (current <= 0 || current >= total) return "";
    // Get the album songs and return the next one's name
    std::string album = controller_.getCurrentAlbum();
    if (album.empty()) return "";
    std::lock_guard lock(mutex_);
    const auto* a = library_.findAlbumByTitle(album);
    if (!a) return "";
    const auto& songs = a->getSongs();
    if (current < static_cast<int>(songs.size())) {
        return songs[current].getTitle(); // current is 1-based, so index = current
    }
    return "";
}

void LibraryService::setVolume(int percent) {
    controller_.setVolume(percent);
}

int LibraryService::getVolume() const {
    return controller_.getVolume();
}

std::vector<std::string> LibraryService::listOutputDevices() const {
    return PlaybackController::listOutputDevices();
}

void LibraryService::setOutputDevice(int deviceIndex) {
    auto devices = PlaybackController::listOutputDevices();
    if (deviceIndex < 0 || deviceIndex >= static_cast<int>(devices.size())) {
        return;
    }
    controller_.setOutputDevice(devices[deviceIndex]);
}
