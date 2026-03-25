#include "Visuals.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>

#include "../service/ILibraryService.h"
#include "Palette.h"

ftxui::Component CreateVisualsCircular(ILibraryService& service) {
  using namespace ftxui;

  auto displayBars = std::make_shared<std::vector<float>>();
  auto frameCount = std::make_shared<int>(0);

  return Renderer([&service, displayBars, frameCount] {
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

    (*frameCount)++;
    float phase = (float)(*frameCount) * 0.08f;

    const auto& gradient = Palette::getCurrentGradient();
    auto gradLeft = LinearGradient();
    auto gradRight = LinearGradient();
    for (size_t i = 0; i < gradient.size(); i++) {
      float stop = static_cast<float>(i) / (gradient.size() - 1);
      gradLeft.Stop(gradient[gradient.size() - 1 - i], stop);
      gradRight.Stop(gradient[i], stop);
    }

    // Each bar becomes a horizontal row: gaugeLeft + gaugeRight mirrored
    // Sine modulation creates a flowing wave shape
    Elements rows;
    for (int i = 0; i < barCount; ++i) {
      float amp = (*displayBars)[i];
      // Sine envelope based on bar position + animated phase
      float envelope = std::abs(std::sin((float)i / (float)barCount * 3.14159f + phase));
      float value = std::min(amp * envelope, 1.0f);

      auto row = hbox({
        gaugeLeft(value) | color(gradLeft) | flex,
        text(" "),
        gaugeRight(value) | color(gradRight) | flex,
      });
      rows.push_back(row | yflex);
    }

    return vbox(rows) | yflex | flex;
  });
}
