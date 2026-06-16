#include "VisualsBase.h"
#include "Palette.h"
#include "../service/IVisualizationSource.h"

VisualsBase::VisualsBase(IVisualizationSource& service) : service_(service) {}

const std::vector<ftxui::Color>& VisualsBase::palette() const {
    return Palette::getCurrentGradient();
}

void VisualsBase::smoothBars() {
    auto target = service_.getSpectrumBars();
    int barCount = (int)target.size();
    if ((int)displayBars_.size() != barCount)
        displayBars_.assign(barCount, 0.f);

    float decayRate = 0.04f + service_.getBpmLoadFactor() * 0.36f;
    for (int i = 0; i < barCount; ++i) {
        float& cur = displayBars_[i];
        float tgt = target[i];
        if (tgt > cur) cur = tgt;
        else cur += (tgt - cur) * decayRate;
    }
}
