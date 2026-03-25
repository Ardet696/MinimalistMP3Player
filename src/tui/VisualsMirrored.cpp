#include "Visuals.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>

#include "../service/ILibraryService.h"
#include "Palette.h"

ftxui::Component CreateVisualsMirrored(ILibraryService& service) {
  using namespace ftxui;

  auto displayBars = std::make_shared<std::vector<float>>();

  return Renderer([&service, displayBars] {
    auto targetBars = service.getSpectrumBars();
    int barCount = (int)targetBars.size();
    if (barCount == 0) {
      return text("") | flex;
    }

    if ((int)displayBars->size() != barCount) {
      displayBars->assign(barCount, 0.f);
    }

    float loadFactor = service.getBpmLoadFactor();
    float decayRate = 0.04f + loadFactor * 0.36f;
    for (int i = 0; i < barCount; ++i) {
      float& cur = (*displayBars)[i];
      float tgt = targetBars[i];
      if (tgt > cur) {
        cur = tgt;
      } else {
        cur += (tgt - cur) * decayRate;
      }
    }

    const auto& gradient = Palette::getCurrentGradient();
    auto gradUp = LinearGradient();
    auto gradDown = LinearGradient();
    for (size_t i = 0; i < gradient.size(); i++) {
      float stop = static_cast<float>(i) / (gradient.size() - 1);
      gradUp.Stop(gradient[i], stop);
      gradDown.Stop(gradient[gradient.size() - 1 - i], stop);
    }

    // Top half: gaugeDown (bars grow downward from top toward center)
    Elements topBars;
    topBars.reserve(barCount * 2 - 1);
    for (int i = 0; i < barCount; ++i) {
      topBars.push_back(gaugeDown((*displayBars)[i]) | xflex_grow | yflex);
      if (i < barCount - 1) {
        topBars.push_back(text(" "));
      }
    }

    // Bottom half: gaugeUp (bars grow upward from bottom toward center)
    Elements bottomBars;
    bottomBars.reserve(barCount * 2 - 1);
    for (int i = 0; i < barCount; ++i) {
      bottomBars.push_back(gaugeUp((*displayBars)[i]) | xflex_grow | yflex);
      if (i < barCount - 1) {
        bottomBars.push_back(text(" "));
      }
    }

    auto topLayer = hbox(topBars) | color(gradDown) | yflex;
    auto bottomLayer = hbox(bottomBars) | color(gradUp) | yflex;

    return vbox({
      topLayer,
      bottomLayer,
    }) | border | flex;
  });
}
