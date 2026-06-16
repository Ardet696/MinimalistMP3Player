#pragma once
#include <ftxui/component/component.hpp>
#include <memory>
class IVisualizationSource;

ftxui::Component CreatePlayingBar(IVisualizationSource& service, std::shared_ptr<int> visualIndex);
