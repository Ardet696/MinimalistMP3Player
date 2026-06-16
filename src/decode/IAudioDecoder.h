#ifndef MP3PLAYER_IAUDIODECODER_H
#define MP3PLAYER_IAUDIODECODER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>

class IAudioDecoder {
public:
    virtual ~IAudioDecoder() = default;

    virtual bool open(const std::filesystem::path& filePath) = 0;
    virtual void close() = 0;

    virtual bool isOpen() const = 0;
    virtual int sampleRate() const = 0;
    virtual int channels() const = 0;
    virtual std::uint64_t totalSamples() const = 0;

    virtual std::size_t decodeFrames(std::span<int16_t> outInterleaved, std::size_t outFrames) = 0;

    virtual const std::string& lastError() const = 0;
};

#endif // MP3PLAYER_IAUDIODECODER_H
