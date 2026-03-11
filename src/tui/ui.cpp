#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

#include "FileManager.h"
#include "Visuals.h"
#include "PlayingBar.h"
#include "UserInputs.h"
#include "../library/MusicLibrary.h"
#include "../service/LibraryService.h"
#include "../service/ConfigService.h"
#include "../player/PlaybackController.h"

int main() {
  using namespace ftxui;

  // Redirect stderr to a log file so prints don't corrupt the TUI.
  // stdout is left alone — FTXUI uses it for rendering.
  static std::ofstream logFile("/tmp/mp3player.log", std::ios::trunc);
  std::cerr.rdbuf(logFile.rdbuf());

  ConfigService config;
  MusicLibrary library;
  PlaybackController controller;
  LibraryService service(library, controller);

  // Load persisted root path if it exists
  std::string savedRoot = config.getRootPath();
  if (!savedRoot.empty()) {
    std::string error;
    service.setRootPath(savedRoot, error);
  }

  auto reload_flag = std::make_shared<bool>(false);

  auto screen = ScreenInteractive::Fullscreen();
  auto file_manager = CreateFileManager(service, reload_flag);
  auto visuals      = CreateVisuals();
  auto playing_bar  = CreatePlayingBar(service, reload_flag);
  auto user_inputs  = CreateUserInputs(service, config, reload_flag);

  auto layout = Container::Vertical({
    Container::Horizontal({file_manager, visuals}),
    playing_bar,
    user_inputs,
  });

  auto renderer = Renderer(layout, [&] {
    return hbox({
      file_manager->Render() | size(WIDTH, EQUAL, 100) | flex_grow | yflex,
      vbox({
        visuals->Render() | size(HEIGHT, EQUAL, 40) | flex_grow,
        playing_bar->Render() | size(HEIGHT, EQUAL, 30) | flex_grow,
        user_inputs->Render() | size(HEIGHT, EQUAL, 10) | flex_grow,
      }) | size(WIDTH, EQUAL, 300) | flex_grow | yflex,
    }) | border | flex;
  });

  auto component = CatchEvent(renderer, [&](Event event) {
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

  // Background refresh thread will posts a custom event 30 times/second to keep UI updated
  std::atomic<bool> running{true};
  std::thread refresh([&] {
    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
      screen.PostEvent(Event::Custom);
    }
  });

  screen.Loop(component);

  running = false;
  refresh.join();

  return EXIT_SUCCESS;
}
