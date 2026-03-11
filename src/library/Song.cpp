#include "Song.h"
#include "../decode/Mp3Decoder.h"
#include "../config/Config.h"
#include <vector>
#include <sstream>
#include <iomanip>

Song::Song(const std::filesystem::path& filePath, const std::string& albumName)
    : filePath_(filePath) , title_(filePath.stem().string())  // Use filename without extension as title
    , artist_("Unknown")
     , album_(albumName)
      , type_(SongType::Standard)
     , durationSeconds_(0)
{
    extractMetadata();
}

void Song::extractMetadata() {
    // This is 100-1000x faster than decoding the entire file!

    // Get file size
    if (!std::filesystem::exists(filePath_)) {
        durationSeconds_ = 0;
        type_ = SongType::Standard;
        return;
    }

    const std::uintmax_t fileSizeBytes = std::filesystem::file_size(filePath_);

    // Estimate bitrate by sampling first 10 frames 200IQ, open decoder just to get sample rate and channels
    Mp3Decoder decoder;
    if (!decoder.open(filePath_)) {
        durationSeconds_ = 0;
        type_ = SongType::Standard;
        return;
    }

    const int sampleRate = decoder.sampleRate();
    const int channels = decoder.channels();

    const std::size_t sampleFrames = 10;
    const std::size_t chunkFrames = 1152;
    std::vector<int16_t> buffer(chunkFrames * channels);

    std::size_t totalFramesSampled = 0;
    for (std::size_t i = 0; i < sampleFrames; ++i) {
        const std::size_t framesDecoded = decoder.decodeFrames(
            std::span<int16_t>(buffer.data(), buffer.size()),
            chunkFrames
        );
        if (framesDecoded == 0) break;
        totalFramesSampled += framesDecoded;
    }

    decoder.close();

    if (totalFramesSampled == 0 || sampleRate == 0) {
        durationSeconds_ = 0;
        type_ = SongType::Standard;
        return;
    }

    // Estimate duration using file size and average bitrate, typical for mp3 is  128-320 kbps, I assume 192 kbps as default
    // PCM samples per second = sampleRate * channels
    // File size in bytes = (total_samples * compression_ratio) / sample_rate

    const int estimatedDurationSeconds = static_cast<int>(
        (fileSizeBytes * 8.0) / Config::ESTIMATED_MP3_BITRATE
    );

    durationSeconds_ = estimatedDurationSeconds;

    // Classify song type based on duration
    type_ = classifyByDuration(durationSeconds_);
}

Song::SongType Song::classifyByDuration(int durationSeconds) {
    if (durationSeconds < 120) {
        return SongType::Interlude;
    } else if (durationSeconds < 420) {
        return SongType::Standard;
    } else if (durationSeconds < 900) {
        return SongType::Suite;
    } else {
        return SongType::Medley;
    }
}

std::string Song::getTypeAsString() const {
    switch (type_) {
        case SongType::Interlude: return "Interlude";
        case SongType::Standard: return "Standard";
        case SongType::Suite: return "Suite";
        case SongType::Medley: return "Medley";
        default: return "Unknown";
    }
}

std::string Song::getFormattedDuration() const {
    const int minutes = durationSeconds_ / 60;
    const int seconds = durationSeconds_ % 60;

    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}
