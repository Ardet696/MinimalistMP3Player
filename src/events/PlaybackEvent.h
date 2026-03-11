#ifndef MP3PLAYER_PLAYBACKEVENT_H
#define MP3PLAYER_PLAYBACKEVENT_H

#include <filesystem>
#include <string>

/**
 * PlaybackEvent - Events that occur during playback.
 *
 * Used by PlaybackEventPublisher to notify observers (UI, logging, etc.)
 * of state changes.
 */
enum class PlaybackEventType {
    SongStarted,
    SongPaused,
    SongResumed,
    SongStopped,
    SongFinished,
    AlbumFinished,
    TrackChanged,
    LoadFailed
};

struct PlaybackEvent {
    PlaybackEventType type;
    std::filesystem::path songPath;
    int trackNumber{0};
    int totalTracks{0};
    std::string message;

    PlaybackEvent(const PlaybackEventType t) : type(t) {}

    PlaybackEvent(const PlaybackEventType t, const std::filesystem::path& path, const int trackNum = 0, const int total = 0, std::string msg = "")
    : type(t), songPath(path), trackNumber(trackNum), totalTracks(total), message(std::move(msg)) {}
};

#endif
