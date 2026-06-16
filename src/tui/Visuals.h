#pragma once
#include <ftxui/component/component.hpp>
#include <memory>

class IVisualizationSource;

ftxui::Component CreateVisuals(IVisualizationSource& service);
ftxui::Component CreateVisualsMirrored(IVisualizationSource& service);
ftxui::Component CreateVisualsRolling(IVisualizationSource& service);
ftxui::Component CreateVisualsCircular(IVisualizationSource& service);
ftxui::Component CreateVisualsPanel(IVisualizationSource& service, std::shared_ptr<int> activeIndex);
