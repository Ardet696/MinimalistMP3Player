#ifndef MP3PLAYER_UI_H
#define MP3PLAYER_UI_H

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>

class ILibraryService;
class IConfigService;

ftxui::Component buildTui(
    ILibraryService& service,
    IConfigService& config,
    ftxui::ScreenInteractive& screen,
    std::shared_ptr<bool> reloadFlag,
    std::shared_ptr<int> visualIndex);

#endif
