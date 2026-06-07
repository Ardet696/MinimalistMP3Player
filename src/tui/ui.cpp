#include "ui.h"

#include <ftxui/dom/elements.hpp>

#include "FileManager.h"
#include "PlayingBar.h"
#include "UserInputs.h"

ftxui::Component buildTui(
    ILibraryService& service,
    IConfigService& config,
    ftxui::ScreenInteractive& screen,
    std::shared_ptr<bool> reloadFlag,
    std::shared_ptr<int> visualIndex)
{
  using namespace ftxui;

  auto file_manager = CreateFileManager(service, reloadFlag);
  auto playing_bar  = CreatePlayingBar(service, visualIndex);
  auto user_inputs  = CreateUserInputs(service, config, reloadFlag, visualIndex);

  auto layout = Container::Vertical({
    file_manager,
    playing_bar,
    user_inputs,
  });

  auto renderer = Renderer(layout, [=] {
    return hbox({
      file_manager->Render() | size(WIDTH, EQUAL, 100) | flex_grow | yflex,
      vbox({
        playing_bar->Render() | size(HEIGHT, EQUAL, 20) | flex_grow,
        user_inputs->Render() | size(HEIGHT, EQUAL, 20) | flex_grow,
      }) | size(WIDTH, EQUAL, 300) | flex_grow | yflex,
    }) | border | flex;
  });

  return CatchEvent(renderer, [&screen, user_inputs](Event event) {
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    if (event.is_character()) {
      std::string ch = event.character();
      if (!ch.empty() && ch[0] >= 32 && ch[0] < 127) {
        user_inputs->TakeFocus();
      }
    }
    return false;
  });
}
