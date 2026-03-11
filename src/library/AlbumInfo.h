#ifndef MP3PLAYER_ALBUMINFO_H
#define MP3PLAYER_ALBUMINFO_H

#include <string>
#include <vector>
#include <filesystem>

/**
 * AlbumInfo - Lightweight album metadata (no song loading at construction).
 *
 *   - Song names loaded only when user selects this album
 *   - No MP3 decoding, no duration calculation, no heavy IO
 *
 */
class AlbumInfo {
public:

    explicit AlbumInfo(const std::filesystem::path& dirPath);

    std::string getTitle() const { return title_; }
    std::filesystem::path getPath() const { return dirPath_; }
    bool isSongsLoaded() const { return songsLoaded_; }
    int getNumSongs() const { return static_cast<int>(songFilenames_.size()); }
    const std::vector<std::string>& getSongFilenames() const { return songFilenames_; }

    /**
     * Lazily load song filenames from directory, call this only when user selects this album.
     * Returns number of songs found.
     */
    int loadSongNames();
    std::filesystem::path getSongPath(int index) const;

private:
    std::filesystem::path dirPath_;
    std::string title_;
    std::vector<std::string> songFilenames_;  // Pure File names
    bool songsLoaded_;
};

#endif
