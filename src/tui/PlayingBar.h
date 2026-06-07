#pragma once
#include <ftxui/component/component.hpp>
#include <memory>
class ILibraryService;

ftxui::Component CreatePlayingBar(ILibraryService& service, std::shared_ptr<int> visualIndex);
