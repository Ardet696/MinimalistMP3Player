#include "Visuals.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>

#include "../service/ILibraryService.h"
#include "Palette.h"

ftxui::Component CreateVisuals(ILibraryService& service) {
  using namespace ftxui;

  auto displayMags = std::make_shared<std::vector<float>>();
  auto runningPeak = std::make_shared<float>(0.001f);

  // Pre compute logarithmic bin mapping (done once per resize)
  auto logBins = std::make_shared<std::vector<float>>();
  auto lastWidth = std::make_shared<int>(0);

  return Renderer([&service, displayMags, runningPeak, logBins, lastWidth] {
    auto mags = service.getSpectrumMagnitudes();
    if (mags.empty()) {
      return text("") | flex;
    }

    // Smooth the magnitudes
    if (displayMags->size() != mags.size()) {
      displayMags->assign(mags.size(), 0.f);
    }
    for (size_t i = 0; i < mags.size(); ++i) {
      float& cur = (*displayMags)[i];
      float tgt = mags[i];
      if (tgt > cur) {
        cur += (tgt - cur) * 0.3f;
      } else {
        cur += (tgt - cur) * 0.12f;
      }
    }

    // Adaptive normalization
    float framePeak = *std::max_element(displayMags->begin(), displayMags->end());
    if (framePeak > *runningPeak) {
      *runningPeak = framePeak;
    } else {
      *runningPeak += (framePeak - *runningPeak) * 0.01f;
    }
    float normFactor = 1.0f / std::max(*runningPeak, 0.001f);

    const auto& gradient = Palette::getCurrentGradient();

    return canvas([&gradient, curMags = *displayMags, normFactor,
                   logBins, lastWidth](Canvas& c) {
      int w = c.width();
      int h = c.height();
      if (w <= 0 || h <= 0) return;

      int centerY = h / 2;
      int binCount = (int)curMags.size();
      if (binCount < 2) return;

      // Rebuild logarithmic bin lookup when width changes
      int usableBins = binCount / 2;  // Only lower half of FFT is useful
      if (*lastWidth != w) {
        *lastWidth = w;
        logBins->resize(w);
        for (int x = 0; x < w; ++x) {
          // Log mapping: spreads bass across more pixels
          float t = (float)x / (float)(w - 1);
          float logIdx = std::pow(t, 2.0f) * (float)(usableBins - 1);
          (*logBins)[x] = logIdx;
        }
      }

      // Sample amplitude at each x using log mapped bins with interpolation
      auto sampleAmplitude = [&](int x) -> float {
        float logIdx = (*logBins)[x];
        int bin0 = (int)logIdx;
        int bin1 = std::min(bin0 + 1, usableBins - 1);
        float frac = logIdx - (float)bin0;

        float raw = curMags[bin0] * (1.0f - frac) + curMags[bin1] * frac;

        int radius = std::max(1, w / 80);
        float sum = raw;
        int count = 1;
        for (int r = 1; r <= radius; ++r) {
          if (x - r >= 0) { sum += curMags[std::min((int)(*logBins)[x - r], usableBins - 1)]; count++; }
          if (x + r < w)  { sum += curMags[std::min((int)(*logBins)[x + r], usableBins - 1)]; count++; }
        }
        raw = sum / (float)count;

        // Normalize and compress (sqrt boosts quieter frequencies)
        float normalized = std::min(raw * normFactor, 1.0f);
        return std::sqrt(normalized);
      };

      int gSize = (int)gradient.size();

      for (int x = 0; x < w; ++x) {
        float amplitude = sampleAmplitude(x);
        float envelope = std::sin((float)x / (float)w * 3.14159f);
        int y = centerY - (int)(amplitude * envelope * centerY * 0.75f);
        y = std::clamp(y, 0, h - 1);

        int thickness = 1 + (int)(amplitude * 14.0f);
        thickness = std::clamp(thickness, 1, 15);

        for (int p = 0; p < thickness; ++p) {
          int py = y - p;
          if (py < 0) break;
          int colorIdx = (thickness - 1 - p) * (gSize - 1) / std::max(thickness - 1, 1);
          c.DrawPoint(x, py, true, gradient[std::clamp(colorIdx, 0, gSize - 1)]);
        }
      }

      for (int x = 0; x < w; ++x) {
        float amplitude = sampleAmplitude(x);
        float envelope = std::sin((float)x / (float)w * 3.14159f);
        int y = centerY + (int)(amplitude * envelope * centerY * 0.75f);
        y = std::clamp(y, 0, h - 1);

        int thickness = 1 + (int)(amplitude * 14.0f);
        thickness = std::clamp(thickness, 1, 15);

        for (int p = 0; p < thickness; ++p) {
          int py = y + p;
          if (py >= h) break;
          int colorIdx = (thickness - 1 - p) * (gSize - 1) / std::max(thickness - 1, 1);
          c.DrawPoint(x, py, true, gradient[std::clamp(colorIdx, 0, gSize - 1)]);
        }
      }

      for (int x = 0; x < w; x += 4) {
        c.DrawPoint(x, centerY, true, Color::GrayDark);
      }
    }) | flex;
  });
}
