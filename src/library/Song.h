#ifndef MP3PLAYER_SONG_H
#define MP3PLAYER_SONG_H

#include <string>
#include <filesystem>


class Song {
public:
    enum class SongType {
        Interlude,
        Standard,
        Suite,
        Medley
    };

    /**
     * Create a Song from an MP3 file path.
     * Automatically determines song type based on duration.
     *
     * @param filePath - Path to the .mp3 file
     * @param albumName - Name of the album (from directory name)
     */
    Song(const std::filesystem::path& filePath, const std::string& albumName);

    std::filesystem::path getFilePath() const { return filePath_; }
    std::string getTitle() const { return title_; }
    std::string getAlbum() const { return album_; }
    std::string getArtist() const { return artist_; }
    SongType getType() const { return type_; }
    int getDurationSeconds() const { return durationSeconds_; }

    void setTitle(const std::string& title) { title_ = title; }
    void setArtist(const std::string& artist) { artist_ = artist; }
    void setAlbum(const std::string& album) { album_ = album; }

    std::string getTypeAsString() const;
    std::string getFormattedDuration() const;

private:

    static SongType classifyByDuration(int durationSeconds);

    void extractMetadata();

    std::filesystem::path filePath_;
    std::string title_;
    std::string artist_;
    std::string album_;
    SongType type_;
    int durationSeconds_;
};

#endif
