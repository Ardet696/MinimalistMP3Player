#ifndef MP3PLAYER_ALBUM_H
#define MP3PLAYER_ALBUM_H

#include <string>
#include <vector>
#include <filesystem>
#include "Song.h"

/**
 * Album - Represents a music album containing songs from a single directory.
 *
 * Rules:
 *   - Only contains .mp3 files from a single directory
 *   - Cannot contain subdirectories
 *   - All songs automatically tagged with album name from directory
 */
class Album {
public:
    enum class AlbumType {
        Single,
        EP,
        StudioAlbum
    };

    explicit Album(const std::filesystem::path& dirPath);

    // Getters
    std::string getTitle() const { return title_; }
    std::string getArtist() const { return artist_; }
    AlbumType getType() const { return type_; }
    int getNumSongs() const { return static_cast<int>(songs_.size()); }
    const std::vector<Song>& getSongs() const { return songs_; }
    std::filesystem::path getPath() const { return dirPath_; }

    void setTitle(const std::string& title) { title_ = title; }
    void setArtist(const std::string& artist) { artist_ = artist; }

    std::string getTypeAsString() const;
    int getTotalDurationSeconds() const;
    std::string getFormattedTotalDuration() const;

    // Song access
    const Song* getSongByIndex(int index) const;

private:

    static AlbumType classifyByNumSongs(int numSongs);
    void loadSongsFromDirectory();

    std::filesystem::path dirPath_;
    std::string title_;
    std::string artist_;
    AlbumType type_;
    std::vector<Song> songs_;
};

#endif
