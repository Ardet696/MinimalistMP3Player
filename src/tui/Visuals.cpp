#include "Visuals.h"
#include <ftxui/dom/elements.hpp>

ftxui::Component CreateVisuals() {
  using namespace ftxui;
  return Renderer([] {
    return text("visuals") | center | border | flex;
  });
}
