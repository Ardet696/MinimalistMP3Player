#include "Fft.h"
#include <cmath>

void Fft::compute(std::vector<cd>& a, bool invert)
{
    int n = static_cast<int>(a.size());
    if (n == 1) return;

    // Bit-reversal permutation (in-place)
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    // Iterative Cooley-Tukey with precomputed twiddle factor per stage
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * M_PI / len * (invert ? -1 : 1);
        cd wn(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = a[i + j];
                cd v = a[i + j + len / 2] * w;
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
                w *= wn;
            }
        }
    }

    if (invert) {
        for (auto& x : a) x /= n;
    }
}
