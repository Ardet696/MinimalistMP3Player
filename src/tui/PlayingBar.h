#pragma once
#include <ftxui/component/component.hpp>
#include <memory>
class ILibraryService;

ftxui::Component CreatePlayingBar(ILibraryService& service, const std::shared_ptr<bool>& reload_flag,
                                  std::shared_ptr<int> visualIndex);
