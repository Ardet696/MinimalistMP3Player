#pragma once
#include <ftxui/component/component.hpp>
class ILibraryService;

ftxui::Component CreatePlayingBar(ILibraryService& service, const std::shared_ptr<bool>& reload_flag);
