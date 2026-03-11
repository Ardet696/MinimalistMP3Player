#include "Playlist.h"
#include <iostream>
#include <algorithm>

Playlist::Playlist(const std::filesystem::path& dirPath, const std::string& creator)
    : dirPath_(dirPath)
    , title_(dirPath.filename().string())  // Directory name as playlist title
    , creator_(creator)
{
    loadSongsFromDirectory();
}

void Playlist::loadSongsFromDirectory() {
    if (!std::filesystem::exists(dirPath_) || !std::filesystem::is_directory(dirPath_)) {
        std::cerr << "Playlist directory does not exist or is not a directory: " << dirPath_ << "\n";
        return;
    }

    // Iterate through directory entries (non recursive)
    for (const auto& entry : std::filesystem::directory_iterator(dirPath_)) {
        // Skip subdirectories - playlists can only contain files
        if (entry.is_directory()) {
            continue;
        }

        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            try {
                // Create Song object with "Playlist" as album name
                // (actual album is unknown unless we parse ID3 tags)
                songs_.emplace_back(entry.path(), title_);
            } catch (const std::exception& e) {
                std::cerr << "Failed to load song " << entry.path() << ": " << e.what() << "\n";
                // Continue with next file
            }
        }
    }

    // Sort songs by filename for consistent ordering
    std::sort(songs_.begin(), songs_.end(), [](const Song& a, const Song& b) {
        return a.getFilePath().filename() < b.getFilePath().filename();
    });
}

int Playlist::getTotalDurationSeconds() const {
    int total = 0;
    for (const auto& song : songs_) {
        total += song.getDurationSeconds();
    }
    return total;
}

std::string Playlist::getFormattedTotalDuration() const {
    const int totalSeconds = getTotalDurationSeconds();
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds % 3600) / 60;
    const int seconds = totalSeconds % 60;

    if (hours > 0) {
        return std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    } else {
        return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    }
}

const Song* Playlist::getSongByIndex(int index) const {
    if (index < 0 || index >= static_cast<int>(songs_.size())) {
        return nullptr;
    }
    return &songs_[index];
}
