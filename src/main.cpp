#include <ftxui/component/screen_interactive.hpp>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <set>
#include <thread>

#include "tui/ui.h"
#include "tui/Palette.h"
#include "library/MusicLibrary.h"
#include "service/LibraryService.h"
#include "service/ConfigService.h"
#include "player/PlaybackController.h"
#include "events/NotificationBus.h"

static std::set<std::string> snapshotDirectory(const std::string& root) {
  std::set<std::string> entries;
  if (root.empty()) return entries;
  std::error_code ec;
  for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
    if (entry.is_directory(ec)) {
      entries.insert(entry.path().filename().string());
    }
  }
  return entries;
}

int main() {
  using namespace ftxui;

  NotificationBus bus;
  ConfigService config;
  MusicLibrary library;
  PlaybackController controller(bus);
  LibraryService service(library, controller, bus);

  // Load persisted config
  std::string savedRoot = config.getRootPath();
  if (!savedRoot.empty()) {
    std::string error;
    service.setRootPath(savedRoot, error);
  }

  int savedTheme = config.getTheme();
  if (savedTheme >= 0 && savedTheme <= 3)
    Palette::setGradient(static_cast<Palette::Theme>(savedTheme));

  // Shared state between UI and background threads
  auto reloadFlag  = std::make_shared<std::atomic<bool>>(false);
  auto visualIndex = std::make_shared<int>(config.getVisual());

  auto screen    = ScreenInteractive::Fullscreen();
  auto component = buildTui(service, config, screen, reloadFlag, visualIndex);

  std::atomic<bool> running{true};

  std::thread refresh([&] {
    while (running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
      screen.PostEvent(Event::Custom);
    }
  });

  std::thread dirWatcher([&] {
    auto lastSnapshot = snapshotDirectory(config.getRootPath());
    while (running) {
      for (int i = 0; i < 20 && running; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (!running) break;

      std::string root = config.getRootPath();
      if (root.empty()) continue;

      auto currentSnapshot = snapshotDirectory(root);
      if (currentSnapshot != lastSnapshot) {
        lastSnapshot = currentSnapshot;
        std::string error;
        service.setRootPath(root, error);
        *reloadFlag = true;
        screen.PostEvent(Event::Custom);
      }
    }
  });

  screen.Loop(component);
  running = false;
  controller.stop();
  refresh.join();
  dirWatcher.join();

  return EXIT_SUCCESS;
}
