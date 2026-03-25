#include "Visuals.h"
#include <vector>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "../service/ILibraryService.h"

ftxui::Component CreateVisualsPanel(ILibraryService& service, std::shared_ptr<int> activeIndex) {
  using namespace ftxui;

  std::vector<Component> visualizers = {
    CreateVisuals(service),          // 0: Oscilloscope
    CreateVisualsMirrored(service),  // 1: Mirrored spectrum
    CreateVisualsRolling(service),   // 2: Rolling waveform
    CreateVisualsCircular(service),  // 3: Circular spectrum
  };

  auto container = Container::Tab(visualizers, activeIndex.get());

  return Renderer(container, [=] {
    return visualizers[*activeIndex]->Render();
  });
}
