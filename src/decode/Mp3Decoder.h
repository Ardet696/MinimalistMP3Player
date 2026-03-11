#ifndef MP3PLAYER_MP3DECODER_H
#define MP3PLAYER_MP3DECODER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

/**
 * MP3 -> PCM decoder (int16_t) using minimp3.
 *
 * Output format:
 * - samples are interleaved (L,R,L,R,...) when channels()==2
 * - sample type is int16_t
 *
 * Units:
 * - 1 frame = 1 sample per channel
 */
class Mp3Decoder {
public:
    Mp3Decoder();
    ~Mp3Decoder();

    // Resource owning type: non-copyable, movable.
    Mp3Decoder(const Mp3Decoder&) = delete;
    Mp3Decoder& operator=(const Mp3Decoder&) = delete;
    Mp3Decoder(Mp3Decoder&&) noexcept;
    Mp3Decoder& operator=(Mp3Decoder&&) noexcept;

bool open(const std::filesystem::path& filePath);
    void close();

    bool isOpen() const;
    int sampleRate() const; // Hz
    int channels() const;   // 1 or 2
    std::uint64_t totalSamples() const; // total samples in file (channels included)

    // Decode up to outFrames into outInterleaved.
    // outInterleaved must have at least outFrames * channels() samples.
    // Returns number of frames produced; 0 means end-of-stream.
std::size_t decodeFrames(std::span<int16_t> outInterleaved, std::size_t outFrames);

const std::string& lastError() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

#endif // MP3PLAYER_MP3DECODER_H
