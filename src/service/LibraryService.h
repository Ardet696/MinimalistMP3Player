#ifndef MP3PLAYER_LIBRARYSERVICE_H
#define MP3PLAYER_LIBRARYSERVICE_H

#include "ILibraryService.h"
#include "../library/MusicLibrary.h"
#include "../library/LibraryScanner.h"
#include "../player/PlaybackController.h"
#include <mutex>

class LibraryService : public ILibraryService {
public:
    explicit LibraryService(MusicLibrary& library, PlaybackController& controller);

    std::vector<std::string> getAlbumNames() const override;
    std::vector<std::string> getSongNames(const std::string& album) const override;
    std::vector<std::vector<std::string>> getAllSongNames() const override;
    bool setRootPath(const std::string& path, std::string& outError) override;
    void playSong(const std::string& album, int songIndex) override;
    void prevSong() override;
    void nextSong() override;
    void resumePlayback() override;
    void stopPlayback() override;
    std::vector<float> getSpectrumBars() const override;
    std::vector<float> getSpectrumMagnitudes() const override;
    float getPlaybackProgress() const override;
    float getBpmLoadFactor() const override;
    std::string getCurrentAlbum() const override;
    int getCurrentTrackIndex() const override;
    std::vector<std::string> listOutputDevices() const override;
    void setOutputDevice(int deviceIndex) override;
private:
    MusicLibrary& library_;
    mutable std::mutex mutex_;
    LibraryScanner scanner_;
    PlaybackController& controller_;
};

#endif // MP3PLAYER_LIBRARYSERVICE_H
