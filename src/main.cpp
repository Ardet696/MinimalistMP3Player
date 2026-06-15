#include <ftxui/component/app.hpp>
#include <atomic>
#include <memory>

#include "tui/ui.h"
#include "tui/Palette.h"
#include "tui/ScreenRefreshTimer.h"
#include "util/DirectoryWatcher.h"
#include "library/MusicLibrary.h"
#include "service/LibraryService.h"
#include "service/AsyncLibraryService.h"
#include "service/ConfigService.h"
#include "player/PlaybackController.h"
#include "events/NotificationBus.h"
#include "commands/CommandQueue.h"

int main() {
  NotificationBus bus;
  ConfigService config;
  MusicLibrary library;
  CommandQueue cmdQueue;
  PlaybackController controller(bus, cmdQueue);
  LibraryService service(library, controller, bus);
  AsyncLibraryService asyncService(service, cmdQueue);

  std::string savedRoot = config.getRootPath();
  if (!savedRoot.empty()) {
    std::string error;
    service.setRootPath(savedRoot, error);
  }

  int savedTheme = config.getTheme();
  if (savedTheme >= 0 && savedTheme <= 3)
    Palette::setGradient(static_cast<Palette::Theme>(savedTheme));

  auto reloadFlag  = std::make_shared<std::atomic<bool>>(false);
  int savedVisual  = config.getVisual();
  if (savedVisual < 0 || savedVisual > 4) savedVisual = 0;
  auto visualIndex = std::make_shared<int>(savedVisual);

  auto screen    = ftxui::App::Fullscreen();
  auto component = buildTui(asyncService, config, screen, reloadFlag, visualIndex);

  ScreenRefreshTimer refreshTimer(screen, std::chrono::milliseconds(33));

  DirectoryWatcher dirWatcher(config, [&](const std::string& root) {
    std::string error;
    service.setRootPath(root, error);
    *reloadFlag = true;
    screen.PostEvent(ftxui::Event::Custom);
  });

  screen.Loop(component);

  return EXIT_SUCCESS;
}
