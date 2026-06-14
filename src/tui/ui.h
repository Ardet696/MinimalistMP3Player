#ifndef MP3PLAYER_UI_H
#define MP3PLAYER_UI_H

#include <ftxui/component/component.hpp>
#include <ftxui/component/app.hpp>
#include <atomic>
#include <memory>

class ILibraryService;
class IConfigService;

ftxui::Component buildTui(
    ILibraryService& service,
    IConfigService& config,
    ftxui::App& screen,
    std::shared_ptr<std::atomic<bool>> reloadFlag,
    std::shared_ptr<int> visualIndex);

#endif
