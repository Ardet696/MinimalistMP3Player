#include "PlayingBar.h"
#include "Visuals.h"
#include "Palette.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>

#include "../service/IVisualizationSource.h"

ftxui::Component CreatePlayingBar(IVisualizationSource& service, std::shared_ptr<int> visualIndex)
{
  using namespace ftxui;

  auto displayBars = std::make_shared<std::vector<float>>();

  auto vizOscilloscope = CreateVisuals(service);
  auto vizMirrored     = CreateVisualsMirrored(service);
  auto vizRolling      = CreateVisualsRolling(service);
  auto vizCircular     = CreateVisualsCircular(service);

  std::vector<Component> vizChildren = {vizOscilloscope, vizMirrored, vizRolling, vizCircular};
  auto altIndex = std::make_shared<int>(0);
  auto vizTab = Container::Tab(vizChildren, altIndex.get());

  return Renderer(vizTab, [&service, displayBars, visualIndex, altIndex, vizTab]
  {
    float progress   = service.getPlaybackProgress();
    float loadFactor = service.getBpmLoadFactor();

    const auto& gradient = Palette::getCurrentGradient();
    auto grad = LinearGradient();
    for (size_t i = 0; i < gradient.size(); i++) {
      float stop = static_cast<float>(i) / (gradient.size() - 1);
      grad.Stop(gradient[i], stop);
    }

    auto progress_bar = gauge(progress) | color(grad);

    Element vizLayer;

    if (*visualIndex == 0) {
      auto  targetBars = service.getSpectrumBars();
      int   barCount   = static_cast<int>(targetBars.size());

      if (static_cast<int>(displayBars->size()) != barCount) {
        displayBars->assign(barCount, 0.f);
      }

      float decayRate = 0.04f + loadFactor * 0.36f;
      for (int i = 0; i < barCount; ++i) {
        float& cur = (*displayBars)[i];
        float  tgt = targetBars[i];
        if (tgt > cur) {
          cur = tgt;
        } else {
          cur += (tgt - cur) * decayRate;
        }
      }

      Elements spectrum;
      spectrum.reserve(barCount * 2 - 1);
      for (int i = 0; i < barCount; ++i) {
        spectrum.push_back(gaugeUp((*displayBars)[i]) | xflex_grow | yflex);
        if (i < barCount - 1) {
          spectrum.push_back(text(" "));
        }
      }

      vizLayer = spectrum.empty()
        ? text("") | flex
        : hbox(spectrum) | color(grad) | yflex;
    } else {
      *altIndex = *visualIndex - 1;
      vizLayer = vizTab->Render() | yflex;
    }

    return vbox({
      vizLayer,
      separator(),
      progress_bar,
    }) | border;
  });
}
