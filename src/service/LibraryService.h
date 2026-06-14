#ifndef MP3PLAYER_LIBRARYSERVICE_H
#define MP3PLAYER_LIBRARYSERVICE_H

#include "ILibraryService.h"
#include "../library/MusicLibrary.h"
#include "../library/LibraryScanner.h"
#include "../player/PlaybackController.h"
#include <shared_mutex>

class NotificationBus;

class LibraryService : public ILibraryService {
public:
    LibraryService(MusicLibrary& library, PlaybackController& controller, NotificationBus& bus);

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
    std::string getCurrentSongName() const override;
    std::string getNextSongName() const override;
    void setVolume(int percent) override;
    int  getVolume() const override;
    std::vector<std::string> listOutputDevices() const override;
    void setOutputDevice(int deviceIndex) override;
    NotificationBus& getNotificationBus() override;
private:
    MusicLibrary& library_;
    mutable std::shared_mutex mutex_;
    LibraryScanner scanner_;
    PlaybackController& controller_;
    NotificationBus& bus_;
};

#endif // MP3PLAYER_LIBRARYSERVICE_H
