#include <cstddef>
#include <cstdint>
#include <vector>

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_EX_IMPLEMENTATION
#include "../../src/decode/third_party/minimp3/minimp3_ex.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    mp3dec_ex_t dec;
    if (mp3dec_ex_open_buf(&dec, data, size, MP3D_SEEK_TO_SAMPLE) != 0) {
        return 0;
    }

    std::vector<mp3d_sample_t> pcm(4096);
    std::uint64_t decoded = 0;
    for (;;) {
        const std::size_t got = mp3dec_ex_read(&dec, pcm.data(), pcm.size());
        if (got == 0) break;
        decoded += got;
        if (decoded > (1u << 22)) break;  // bound work on pathological inputs
    }

    mp3dec_ex_close(&dec);
    return 0;
}
