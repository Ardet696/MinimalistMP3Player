#include "Mp3Decoder.h"

#include <cassert>
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_EX_IMPLEMENTATION
#include "third_party/minimp3/minimp3_ex.h"

struct Mp3Decoder::Impl {
    mp3dec_ex_t dec{};
    bool open = false;
    int hz = 0;
    int ch = 0;
    std::string err;
};

Mp3Decoder::Mp3Decoder() : impl_(std::make_unique<Impl>()) {}
Mp3Decoder::~Mp3Decoder() = default;

Mp3Decoder::Mp3Decoder(Mp3Decoder&&) noexcept = default;
Mp3Decoder& Mp3Decoder::operator=(Mp3Decoder&&) noexcept = default;

bool Mp3Decoder::open(const std::filesystem::path& filePath) {
    if (!impl_) return false;

    close();
    impl_->err.clear();

    static_assert(sizeof(mp3d_sample_t) == sizeof(int16_t), "minimp3 is not configured for 16-bit samples; adjust decode type.");

    const std::string pathStr = filePath.string();
    const int rc = mp3dec_ex_open(&impl_->dec, pathStr.c_str(), MP3D_SEEK_TO_SAMPLE);

    if (rc != 0) {
        impl_->err = "mp3dec_ex_open failed (rc=" + std::to_string(rc) + ")";
        impl_->open = false;
        return false;
    }

    impl_->hz = impl_->dec.info.hz;
    impl_->ch = impl_->dec.info.channels;
    impl_->open = true;

    if (impl_->hz <= 0 || (impl_->ch != 1 && impl_->ch != 2)) {
        impl_->err = "Invalid audio format reported by decoder.";
        close();
        return false;
    }

    return true;
}

void Mp3Decoder::close() {
    if (!impl_) return;

    if (impl_->open) {
        mp3dec_ex_close(&impl_->dec);
        impl_->open = false;
    }

    impl_->hz = 0;
    impl_->ch = 0;
}

bool Mp3Decoder::isOpen() const {
    return impl_ && impl_->open;
}

int Mp3Decoder::sampleRate() const {
    return impl_ ? impl_->hz : 0;
}

int Mp3Decoder::channels() const {
    return impl_ ? impl_->ch : 0;
}

std::uint64_t Mp3Decoder::totalSamples() const {
    return (impl_ && impl_->open) ? impl_->dec.samples : 0;
}
// decodeFrames() takes frames but minimp3 returns samples
std::size_t Mp3Decoder::decodeFrames(std::span<int16_t> outInterleaved, std::size_t outFrames) {
    if (!isOpen() || outFrames == 0) return 0;

    const std::size_t ch = static_cast<std::size_t>(impl_->ch);
    const std::size_t samplesRequested = outFrames * ch;

    // Caller contract: enough space for frames * channels samples.
    assert(outInterleaved.size() >= samplesRequested);
    const std::size_t samplesRead = mp3dec_ex_read(&impl_->dec, outInterleaved.data(), samplesRequested);

    return samplesRead / ch; // samples to  frames conversion
}

const std::string& Mp3Decoder::lastError() const {
    static const std::string kEmpty;
    return impl_ ? impl_->err : kEmpty;
}
