#ifndef MP3PLAYER_ILIBRARYSERVICE_H
#define MP3PLAYER_ILIBRARYSERVICE_H
#include <string>
#include <vector>

class ILibraryService
{
    public:
        virtual ~ILibraryService() = default;
        virtual std::vector<std::string> getAlbumNames() const = 0;
        virtual std::vector<std::string> getSongNames(const std::string& album) const = 0;
        virtual std::vector<std::vector<std::string>> getAllSongNames() const = 0;
        virtual bool setRootPath(const std::string& path, std::string& outError) = 0;
        virtual void playSong(const std::string& album, int songIndex) = 0;
        virtual void prevSong() = 0;
        virtual void nextSong() = 0;
        virtual void resumePlayback() = 0;
        virtual void stopPlayback() = 0;
        virtual std::vector<float> getSpectrumBars() const = 0;
        virtual std::vector<float> getSpectrumMagnitudes() const = 0; // 512 for audiovisualizer
        virtual float getPlaybackProgress() const = 0;
        virtual float getBpmLoadFactor() const = 0;
        virtual std::string getCurrentAlbum() const = 0;
        virtual int getCurrentTrackIndex() const = 0;
        virtual std::vector<std::string> listOutputDevices() const = 0;
        virtual void setOutputDevice(int deviceIndex) = 0;
};
#endif //MP3PLAYER_ILIBRARYSERVICE_H