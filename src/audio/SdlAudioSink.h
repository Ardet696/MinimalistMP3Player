#ifndef MP3PLAYER_SDLAUDIOSINK_H
#define MP3PLAYER_SDLAUDIOSINK_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "../util/AudioFormat.h"

class SdlAudioSink {
public:
    using FrameProvider = std::function<std::size_t(int16_t* dstInterleaved, std::size_t framesRequested)>;

    SdlAudioSink();
    ~SdlAudioSink();

    SdlAudioSink(const SdlAudioSink&) = delete;
    SdlAudioSink& operator=(const SdlAudioSink&) = delete;

    bool open(const AudioFormat& fmt, FrameProvider provider, const std::string& deviceName = "", int desiredBufferFrames = 2048);
    void start() const;
    void stop() const;
    void close();
    bool isOpen() const;

    void setVolume(int percent);   // 0-100
    int  getVolume() const;

    /// List available audio output device names via SDL.
    static std::vector<std::string> listOutputDevices();

private:
    static void sdlCallback(void* userdata, std::uint8_t* stream, int len);
    void fill(std::uint8_t* stream, int len) const;

    std::uintptr_t device_ = 0; // SDL_AudioDeviceID stored portably
    AudioFormat fmt_{};
    FrameProvider provider_{};
    bool open_ = false;
    std::atomic<int> volume_{100}; // 0-100
};

#endif
