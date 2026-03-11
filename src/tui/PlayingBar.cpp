#include "PlayingBar.h"
#include "Palette.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>

#include "../service/ILibraryService.h"

ftxui::Component CreatePlayingBar(ILibraryService& service, const std::shared_ptr<bool>& reload_flag)
{
  using namespace ftxui;

  auto displayBars = std::make_shared<std::vector<float>>();

  return Renderer([&service, displayBars]
  {
    float progress   = service.getPlaybackProgress();
    float loadFactor = service.getBpmLoadFactor();
    auto  targetBars = service.getSpectrumBars();
    int   barCount   = static_cast<int>(targetBars.size());

    // Resize display bars if bar count changed (e.g. on first render or reload)
    if (static_cast<int>(displayBars->size()) != barCount) {
      displayBars->assign(barCount, 0.f);
    }

    float decayRate = 0.04f + loadFactor * 0.36f;  // [0.04, 0.40] per frame
    for (int i = 0; i < barCount; ++i) {
      float& cur = (*displayBars)[i];
      float  tgt = targetBars[i];
      if (tgt > cur) {
        cur = tgt;                           // Instant rise to peak
      } else {
        cur += (tgt - cur) * decayRate;      // Exponential fall toward target
      }
    }

    const auto& gradient = Palette::getCurrentGradient();
    auto grad = LinearGradient();
    for (size_t i = 0; i < gradient.size(); i++) {
      float stop = static_cast<float>(i) / (gradient.size() - 1);
      grad.Stop(gradient[i], stop);
    }

    Elements spectrum;
    spectrum.reserve(barCount * 2 - 1);
    for (int i = 0; i < barCount; ++i) {
      spectrum.push_back(gaugeUp((*displayBars)[i]) | xflex_grow | yflex);
      if (i < barCount - 1) {
        spectrum.push_back(text(" "));
      }
    }

    auto spectrum_layer = spectrum.empty()
      ? text("") | flex
      : hbox(spectrum) | color(grad) | yflex;
    auto progress_bar   = gauge(progress) | color(grad);

    return vbox({
      spectrum_layer,
      separator(),
      progress_bar,
    }) | border;
  });
}
