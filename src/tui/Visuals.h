#pragma once
#include <ftxui/component/component.hpp>
#include <memory>

class ILibraryService;

ftxui::Component CreateVisuals(ILibraryService& service);
ftxui::Component CreateVisualsMirrored(ILibraryService& service);
ftxui::Component CreateVisualsRolling(ILibraryService& service);
ftxui::Component CreateVisualsCircular(ILibraryService& service);
ftxui::Component CreateVisualsPanel(ILibraryService& service, std::shared_ptr<int> activeIndex);
