#include "Visuals.h"
#include "VisualsBase.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>
#include "../service/IVisualizationSource.h"

class VisualsRolling : public VisualsBase {
public:
    VisualsRolling(IVisualizationSource& service) : VisualsBase(service) {}

    ftxui::Element render() override {
        auto mags = service_.getSpectrumMagnitudes();
        if (mags.empty())
            return ftxui::text("") | ftxui::flex;

        int binCount = (int)mags.size();
        int usable = binCount / 2;

        int bassEnd = usable / 6;
        int midEnd  = usable / 2;
        float bass = 0.f, mid = 0.f, treble = 0.f;
        for (int i = 1; i < usable; ++i) {
            if (i < bassEnd)     bass   += mags[i];
            else if (i < midEnd) mid    += mags[i];
            else                 treble += mags[i];
        }
        bass   /= std::max(bassEnd - 1, 1);
        mid    /= std::max(midEnd - bassEnd, 1);
        treble /= std::max(usable - midEnd, 1);

        float energy = bass * 0.5f + mid * 0.3f + treble * 0.2f;

        if (energy > runningPeak_) runningPeak_ = energy;
        else runningPeak_ += (energy - runningPeak_) * 0.005f;

        float normalized = std::sqrt(std::min(energy / std::max(runningPeak_, 0.001f), 1.0f));

        waveHistory_.push_back(normalized);
        while ((int)waveHistory_.size() > 2000)
            waveHistory_.pop_front();

        const auto& gradient = palette();

        return ftxui::canvas([&gradient, wave = waveHistory_](ftxui::Canvas& c) {
            int w = c.width();
            int h = c.height();
            if (w <= 0 || h <= 0) return;

            int centerY = h / 2;
            int waveSize = (int)wave.size();
            if (waveSize < 2) return;

            ftxui::Color lineColor   = gradient.size() > 1 ? gradient[1] : ftxui::Color::White;
            ftxui::Color mirrorColor = gradient.size() > 2 ? gradient[2] : ftxui::Color::White;
            ftxui::Color fillColor   = gradient.size() > 0 ? gradient[0] : ftxui::Color::GrayDark;

            int startIdx = std::max(0, waveSize - w);
            int prevX = 0, prevYUp = centerY, prevYDown = centerY;

            for (int x = 0; x < w; ++x) {
                int idx = startIdx + x;
                if (idx >= waveSize) break;

                float amp = wave[idx];
                int yUp   = std::clamp(centerY - (int)(amp * centerY * 0.8f), 0, h - 1);
                int yDown = std::clamp(centerY + (int)(amp * centerY * 0.8f), 0, h - 1);

                if (yUp < centerY)   c.DrawPointLine(x, yUp + 1, x, centerY, fillColor);
                if (yDown > centerY) c.DrawPointLine(x, centerY, x, yDown - 1, fillColor);

                if (x > 0) {
                    c.DrawPointLine(prevX, prevYUp, x, yUp, lineColor);
                    c.DrawPointLine(prevX, prevYDown, x, yDown, mirrorColor);
                }

                prevX = x; prevYUp = yUp; prevYDown = yDown;
            }

            for (int x = 0; x < w; x += 4)
                c.DrawPoint(x, centerY, true, ftxui::Color::GrayDark);
        }) | ftxui::flex;
    }

private:
    std::deque<float> waveHistory_;
    float runningPeak_ = 0.001f;
};

ftxui::Component CreateVisualsRolling(IVisualizationSource& service) {
    auto self = std::make_shared<VisualsRolling>(service);
    return ftxui::Renderer([self] { return self->render(); });
}
