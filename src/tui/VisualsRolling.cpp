#include "Visuals.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>

#include "../service/ILibraryService.h"
#include "Palette.h"

ftxui::Component CreateVisualsRolling(ILibraryService& service) {
  using namespace ftxui;

  // Rolling buffer: each entry is the overall energy at that frame
  auto waveHistory = std::make_shared<std::vector<float>>();
  auto runningPeak = std::make_shared<float>(0.001f);

  return Renderer([&service, waveHistory, runningPeak] {
    auto mags = service.getSpectrumMagnitudes();
    if (mags.empty()) {
      return text("") | flex;
    }

    // Compute energy across frequency bands
    int binCount = (int)mags.size();
    int usable = binCount / 2;

    // Split into bass / mid / treble and compute weighted energy
    float bass = 0.f, mid = 0.f, treble = 0.f;
    int bassEnd = usable / 6;      // ~0-700 Hz
    int midEnd = usable / 2;       // ~700-4000 Hz
    for (int i = 1; i < usable; ++i) {
      if (i < bassEnd) bass += mags[i];
      else if (i < midEnd) mid += mags[i];
      else treble += mags[i];
    }
    bass /= std::max(bassEnd - 1, 1);
    mid /= std::max(midEnd - bassEnd, 1);
    treble /= std::max(usable - midEnd, 1);

    // Weighted mix favoring bass
    float energy = bass * 0.5f + mid * 0.3f + treble * 0.2f;

    // Adaptive normalization
    if (energy > *runningPeak) {
      *runningPeak = energy;
    } else {
      *runningPeak += (energy - *runningPeak) * 0.005f;
    }
    float normalized = std::min(energy / std::max(*runningPeak, 0.001f), 1.0f);
    // Compress
    normalized = std::sqrt(normalized);

    waveHistory->push_back(normalized);
    // Keep enough history for the widest possible canvas
    if ((int)waveHistory->size() > 2000) {
      waveHistory->erase(waveHistory->begin());
    }

    const auto& gradient = Palette::getCurrentGradient();

    return canvas([&gradient, wave = *waveHistory](Canvas& c) {
      int w = c.width();
      int h = c.height();
      if (w <= 0 || h <= 0) return;

      int centerY = h / 2;
      int waveSize = (int)wave.size();
      if (waveSize < 2) return;

      Color lineColor = gradient.size() > 1 ? gradient[1] : Color::White;
      Color mirrorColor = gradient.size() > 2 ? gradient[2] : Color::White;
      Color fillColor = gradient.size() > 0 ? gradient[0] : Color::GrayDark;

      // Map the most recent `w` samples across the canvas, scrolling left
      int startIdx = std::max(0, waveSize - w);

      int prevX = 0;
      int prevYUp = centerY;
      int prevYDown = centerY;

      for (int x = 0; x < w; ++x) {
        int idx = startIdx + x;
        if (idx >= waveSize) break;

        float amp = wave[idx];
        int yUp = centerY - (int)(amp * centerY * 0.8f);
        int yDown = centerY + (int)(amp * centerY * 0.8f);
        yUp = std::clamp(yUp, 0, h - 1);
        yDown = std::clamp(yDown, 0, h - 1);

        // Fill between center and waveform for a solid look
        if (yUp < centerY) {
          c.DrawPointLine(x, yUp + 1, x, centerY, fillColor);
        }
        if (yDown > centerY) {
          c.DrawPointLine(x, centerY, x, yDown - 1, fillColor);
        }

        // Draw the waveform lines
        if (x > 0) {
          c.DrawPointLine(prevX, prevYUp, x, yUp, lineColor);
          c.DrawPointLine(prevX, prevYDown, x, yDown, mirrorColor);
        }

        prevX = x;
        prevYUp = yUp;
        prevYDown = yDown;
      }

      // Center line
      for (int x = 0; x < w; x += 4) {
        c.DrawPoint(x, centerY, true, Color::GrayDark);
      }
    }) | flex;
  });
}
