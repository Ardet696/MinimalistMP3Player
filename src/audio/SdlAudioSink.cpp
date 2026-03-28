#include "SdlAudioSink.h"

#include <SDL.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

SdlAudioSink::SdlAudioSink() = default;

SdlAudioSink::~SdlAudioSink() {
    close();
}

bool SdlAudioSink::open(const AudioFormat& fmt, FrameProvider provider, const std::string& deviceName, const int desiredBufferFrames) {
    close();
    // Checks before initializing library SDL
    if (fmt.sampleRate <= 0 || (fmt.channels != 1 && fmt.channels != 2)) {
        std::cerr << "SdlAudioSink: invalid format\n";
        return false;
    }
    if (!provider) {
        std::cerr << "SdlAudioSink: provider is null\n";
        return false;
    }

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            std::cerr << "SDL_InitSubSystem failed: " << SDL_GetError() << "\n";
            return false;
        }
    }

    SDL_AudioSpec desired{};
    desired.freq = fmt.sampleRate;
    desired.format = AUDIO_S16SYS;
    desired.channels = static_cast<Uint8>(fmt.channels);
    desired.samples = static_cast<Uint16>(desiredBufferFrames);
    desired.callback = &SdlAudioSink::sdlCallback;
    desired.userdata = this;

    // Pass device name to SDL (nullptr = system default)
    const char* sdlDeviceName = deviceName.empty() ? nullptr : deviceName.c_str();

    SDL_AudioSpec obtained{};
    const SDL_AudioDeviceID dev = SDL_OpenAudioDevice(sdlDeviceName, 0, &desired, &obtained, 0);
    if (dev == 0) {
        std::cerr << "SDL_OpenAudioDevice failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (obtained.format != AUDIO_S16SYS) {
        std::cerr << "SDL obtained unsupported format\n";
        SDL_CloseAudioDevice(dev);
        return false;
    }

    // Reject mismatch to avoid resampling complexity.
    if (obtained.freq != desired.freq || obtained.channels != desired.channels) {
        std::cerr << "SDL obtained different format (freq/ch). Rejecting for now.\n";
        SDL_CloseAudioDevice(dev);
        return false;
    }

    fmt_ = fmt;
    provider_ = std::move(provider);
    device_ = static_cast<std::uintptr_t>(dev);
    open_ = true;
    return true;
}

void SdlAudioSink::start() const {
    if (!open_) return;
    SDL_PauseAudioDevice(static_cast<SDL_AudioDeviceID>(device_), 0);
}

void SdlAudioSink::stop() const {
    if (!open_) return;
    SDL_PauseAudioDevice(static_cast<SDL_AudioDeviceID>(device_), 1);
}

void SdlAudioSink::close() {
    if (!open_) return;
    SDL_CloseAudioDevice(static_cast<SDL_AudioDeviceID>(device_));
    device_ = 0;
    provider_ = nullptr;
    fmt_ = AudioFormat{};
    open_ = false;
}

bool SdlAudioSink::isOpen() const {
    return open_;
}

void SdlAudioSink::setVolume(int percent) {
    volume_.store(std::clamp(percent, 0, 100), std::memory_order_relaxed);
}

int SdlAudioSink::getVolume() const {
    return volume_.load(std::memory_order_relaxed);
}

std::vector<std::string> SdlAudioSink::listOutputDevices() {
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            std::cerr << "SDL_InitSubSystem failed: " << SDL_GetError() << "\n";
            return {};
        }
    }

    // Collect unique device names (ALSA often reports the same chipset name
    // for multiple HDMI/DP outputs — de-duplicate to avoid confusing listings)
    std::vector<std::string> devices;
    const int count = SDL_GetNumAudioDevices(0); // 0 = output devices
    for (int i = 0; i < count; ++i) {
        const char* name = SDL_GetAudioDeviceName(i, 0);
        if (!name) continue;
        std::string nameStr(name);
        bool duplicate = false;
        for (const auto& existing : devices) {
            if (existing == nameStr) { duplicate = true; break; }
        }
        if (!duplicate) {
            devices.push_back(std::move(nameStr));
        }
    }
    return devices;
}

void SdlAudioSink::sdlCallback(void* userdata, std::uint8_t* stream, const int len) {
    static_cast<SdlAudioSink*>(userdata)->fill(stream, len);
}

void SdlAudioSink::fill(std::uint8_t* stream, const int len) const {
    const std::size_t bytesPerFrame = sizeof(int16_t) * static_cast<std::size_t>(fmt_.channels);
    const std::size_t framesRequested = static_cast<std::size_t>(len) / bytesPerFrame;

    auto* out = reinterpret_cast<int16_t*>(stream);
    const std::size_t samplesRequested = framesRequested * static_cast<std::size_t>(fmt_.channels);

    const std::size_t framesWritten = provider_ ? provider_(out, framesRequested) : 0;
    const std::size_t samplesWritten = framesWritten * static_cast<std::size_t>(fmt_.channels);

    if (samplesWritten < samplesRequested) {
        std::memset(out + samplesWritten, 0, (samplesRequested - samplesWritten) * sizeof(int16_t));
    }

    // Apply volume scaling
    int vol = volume_.load(std::memory_order_relaxed);
    if (vol < 100) {
        // SDL_MIX_MAXVOLUME is 128; scale our 0-100 range
        int sdlVol = vol * SDL_MIX_MAXVOLUME / 100;
        // Clear stream first, then mix scaled audio back
        // We need a temp copy since SDL_MixAudioFormat adds to dst
        std::vector<std::uint8_t> tmp(stream, stream + len);
        std::memset(stream, 0, static_cast<std::size_t>(len));
        SDL_MixAudioFormat(stream, tmp.data(), AUDIO_S16SYS, static_cast<Uint32>(len), sdlVol);
    }
}
