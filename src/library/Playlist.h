#ifndef MP3PLAYER_PLAYLIST_H
#define MP3PLAYER_PLAYLIST_H

#include <string>
#include <vector>
#include <filesystem>
#include "Song.h"

/**
 * Songs retain their original album name from their parent directory.
 *
 * Rules:
 *   - Only contains .mp3 files from a single directory
 *   - Cannot contain subdirectories (become discarded)
 */
class Playlist {
public:
    /**
     * Create a Playlist from a directory path.
     * Scans directory for .mp3 files and creates Song objects.
     *
     * @param dirPath - Path to playlist directory
     * @param creator - Name of the playlist creator (default: "User")
     */
    explicit Playlist(const std::filesystem::path& dirPath, const std::string& creator = "User");

    // Getters
    std::string getTitle() const { return title_; }
    std::string getCreator() const { return creator_; }
    int getNumSongs() const { return static_cast<int>(songs_.size()); }
    const std::vector<Song>& getSongs() const { return songs_; }
    std::filesystem::path getPath() const { return dirPath_; }

    void setTitle(const std::string& title) { title_ = title; }
    void setCreator(const std::string& creator) { creator_ = creator; }

    int getTotalDurationSeconds() const;
    std::string getFormattedTotalDuration() const;

    const Song* getSongByIndex(int index) const;

private:

    void loadSongsFromDirectory();

    std::filesystem::path dirPath_;
    std::string title_;
    std::string creator_;
    std::vector<Song> songs_;
};

#endif
