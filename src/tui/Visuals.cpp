#include "Visuals.h"
#include "VisualsBase.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>
#include "../service/ILibraryService.h"

class VisualsOscilloscope : public VisualsBase {
public:
    explicit VisualsOscilloscope(ILibraryService& service) : VisualsBase(service) {}

    ftxui::Element render() override {
        auto mags = service_.getSpectrumMagnitudes();
        if (mags.empty())
            return ftxui::text("") | ftxui::flex;

        if (displayMags_.size() != mags.size())
            displayMags_.assign(mags.size(), 0.f);

        for (size_t i = 0; i < mags.size(); ++i) {
            float& cur = displayMags_[i];
            float tgt = mags[i];
            if (tgt > cur) cur += (tgt - cur) * 0.3f;
            else           cur += (tgt - cur) * 0.12f;
        }

        float framePeak = *std::max_element(displayMags_.begin(), displayMags_.end());
        if (framePeak > runningPeak_) runningPeak_ = framePeak;
        else runningPeak_ += (framePeak - runningPeak_) * 0.01f;
        float normFactor = 1.0f / std::max(runningPeak_, 0.001f);

        const auto& gradient = palette();

        return ftxui::canvas([&gradient, curMags = displayMags_, normFactor,
                               logBins = &logBins_, lastWidth = &lastWidth_](ftxui::Canvas& c) {
            int w = c.width();
            int h = c.height();
            if (w <= 0 || h <= 0) return;

            int centerY = h / 2;
            int binCount = (int)curMags.size();
            if (binCount < 2) return;

            int usableBins = binCount / 2;
            if (*lastWidth != w) {
                *lastWidth = w;
                logBins->resize(w);
                for (int x = 0; x < w; ++x) {
                    float t = (float)x / (float)(w - 1);
                    (*logBins)[x] = std::pow(t, 2.0f) * (float)(usableBins - 1);
                }
            }

            auto sampleAmplitude = [&](int x) -> float {
                float logIdx = (*logBins)[x];
                int bin0 = (int)logIdx;
                int bin1 = std::min(bin0 + 1, usableBins - 1);
                float raw = curMags[bin0] * (1.0f - (logIdx - bin0)) + curMags[bin1] * (logIdx - bin0);

                int radius = std::max(1, w / 80);
                float sum = raw;
                int count = 1;
                for (int r = 1; r <= radius; ++r) {
                    if (x - r >= 0) { sum += curMags[std::min((int)(*logBins)[x - r], usableBins - 1)]; count++; }
                    if (x + r < w)  { sum += curMags[std::min((int)(*logBins)[x + r], usableBins - 1)]; count++; }
                }
                return std::sqrt(std::min(sum / (float)count * normFactor, 1.0f));
            };

            int gSize = (int)gradient.size();

            for (int x = 0; x < w; ++x) {
                float amplitude = sampleAmplitude(x);
                float envelope = std::sin((float)x / (float)w * 3.14159f);
                int y = centerY - (int)(amplitude * envelope * centerY * 0.75f);
                y = std::clamp(y, 0, h - 1);
                int thickness = std::clamp(1 + (int)(amplitude * 14.0f), 1, 15);
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
                int thickness = std::clamp(1 + (int)(amplitude * 14.0f), 1, 15);
                for (int p = 0; p < thickness; ++p) {
                    int py = y + p;
                    if (py >= h) break;
                    int colorIdx = (thickness - 1 - p) * (gSize - 1) / std::max(thickness - 1, 1);
                    c.DrawPoint(x, py, true, gradient[std::clamp(colorIdx, 0, gSize - 1)]);
                }
            }

            for (int x = 0; x < w; x += 4)
                c.DrawPoint(x, centerY, true, ftxui::Color::GrayDark);
        }) | ftxui::flex;
    }

private:
    std::vector<float> displayMags_;
    float runningPeak_ = 0.001f;
    std::vector<float> logBins_;
    int lastWidth_ = 0;
};

ftxui::Component CreateVisuals(ILibraryService& service) {
    auto self = std::make_shared<VisualsOscilloscope>(service);
    return ftxui::Renderer([self] { return self->render(); });
}
