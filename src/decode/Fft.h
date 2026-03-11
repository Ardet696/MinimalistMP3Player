#ifndef MP3PLAYER_FFT_H
#define MP3PLAYER_FFT_H

#include <vector>
#include <complex>

class Fft // CLASSIC FFT
{
public:
    using cd = std::complex<double>;

    // Size of a must be a power of 2.
    static void compute(std::vector<cd>& a, bool invert = false);
};

#endif //MP3PLAYER_FFT_H
