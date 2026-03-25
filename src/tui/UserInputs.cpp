#include "UserInputs.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>

#include "../service/ILibraryService.h"
#include "../service/IConfigService.h"
#include "ftxui/component/event.hpp"
#include "Palette.h"

namespace {
enum class InputMode { Command, RootConfig, ThemeConfig, OutputDevice, VisualsConfig };   // State machine of input bar
constexpr std::string_view CMD_PLAY  = "play";
constexpr std::string_view CMD_STOP  = "stop";
constexpr std::string_view CMD_NEXT  = "next";
constexpr std::string_view CMD_PREV  = "prev";
constexpr std::string_view CMD_HELP  = "help";
constexpr std::string_view CMD_F_HELP = "fileHelp";
constexpr std::string_view CMD_CONFIG = "rootConfig";
constexpr std::string_view CMD_THEMES = "themes";
constexpr std::string_view CMD_OUTPUT = "output";
constexpr std::string_view CMD_VISUALS = "visuals";
constexpr std::string_view CMD_QUIT  = "quit";

/// Shorten a device name to a recognizable label.
/// PipeWire/ALSA names are often long with the chipset prefix and "Output" suffix,
/// e.g. "Raptor Lake-P/U/H cAVS HDMI / DisplayPort 3 Output" -> "HDMI/DP 3"
///      "Raptor Lake-P/U/H cAVS Speaker"                      -> "Speaker"
///      "Victor's AirPods Pro"                                 -> "Victor's AirPods"
std::string truncateDeviceName(const std::string& name, std::size_t maxChars = 20) {
    if (name.empty()) return name;
    if (name.size() <= maxChars) return name;

    std::string work = name;

    // Strip trailing "Output" — it's noise
    const std::string outputSuffix = " Output";
    if (work.size() > outputSuffix.size() &&
        work.compare(work.size() - outputSuffix.size(), outputSuffix.size(), outputSuffix) == 0) {
        work.erase(work.size() - outputSuffix.size());
    }

    // If "HDMI / DisplayPort" is present, shorten to "HDMI/DP"
    auto hdmiPos = work.find("HDMI / DisplayPort");
    if (hdmiPos != std::string::npos) {
        // Take "HDMI/DP" plus anything after (e.g. " 3")
        std::string tail = work.substr(hdmiPos + 18); // after "HDMI / DisplayPort"
        work = "HDMI/DP" + tail;
        if (work.size() <= maxChars) return work;
    }

    // Fall back: take last two words (the distinguishing part)
    std::istringstream iss(work);
    std::vector<std::string> words;
    std::string w;
    while (iss >> w) words.push_back(w);

    if (words.size() <= 2) return work.substr(0, maxChars);

    std::string result = words[words.size() - 2] + " " + words[words.size() - 1];
    if (result.size() <= maxChars) return result;
    return result.substr(0, maxChars);
}

/// Build the numbered device list string for display.
std::string buildDeviceListPrompt(const std::vector<std::string>& devices) {
    if (devices.empty()) return "No output devices found.";

    std::string result = "Select output: ";
    for (int i = 0; i < static_cast<int>(devices.size()); ++i) {
        result += "[" + std::to_string(i + 1) + "]." + truncateDeviceName(devices[i]);
        if (i + 1 < static_cast<int>(devices.size())) result += " ";
    }
    return result;
}

std::string ProcessCommand(const std::string& cmd) {
  if (cmd == CMD_CONFIG) return "Now type the new root path to your music directory (only directories with .mp3 files will be considered)";
  if (cmd == CMD_PLAY)  return "Playing...";
  if (cmd == CMD_STOP)  return "Stopped";
  if (cmd == CMD_NEXT)  return "Next track";
  if (cmd == CMD_PREV)  return "Previous track";
  if (cmd == CMD_HELP)  return "Commands: play, stop, next, prev, output, visuals, themes, quit/q";
  if (cmd == CMD_F_HELP) return  "In Album Manager: ['↑', '↓': albums/songs,'Enter': select,'Backspace': go back]";
  if (cmd == CMD_THEMES) return "Enter [1..4] for the available themes: { [1]-Fire | [2]-BW | [3]-PurpleRain | [4]-Forest } ";
  if (cmd == CMD_QUIT)  return "Quitting...";
  return "Unknown command: " + cmd;
}

}

ftxui::Component CreateUserInputs(ILibraryService& service, IConfigService& config,
                                  std::shared_ptr<bool> reload_flag,
                                  std::shared_ptr<int> visualIndex) {
  using namespace ftxui;
  auto input_content = std::make_shared<std::string>();
  auto new_root_path = std::make_shared<std::string>();
  auto status_msg    = std::make_shared<std::string>("'help' for Mp3Player commands, 'fileHelp' for Album Manager commands, 'RootConfig' to change /Music root ,'themes' to change color palette.");
  auto mode = std::make_shared<InputMode>(InputMode::Command);
  auto cached_devices = std::make_shared<std::vector<std::string>>();

  auto input_option = InputOption();
  input_option.placeholder = "enter command...";
  auto input = Input(input_content.get(), "", input_option);

  // Using capture by value default for the lambda.
  auto component = CatchEvent(input, [=, &service, &config](Event event) mutable {
    if (event == Event::Return && !input_content->empty()) {
     if (*mode == InputMode::Command) {
       if (*input_content == "RootConfig") {
          *mode = InputMode::RootConfig;
          *status_msg = "MODE: RootConfig - Enter new path:";
          input_content->clear(); // Clear bar for the next input
          return true;
       }
       if (*input_content == "themes")
       {
         *mode = InputMode::ThemeConfig;
         *status_msg = "Enter [1..4] for the available themes: { [1]-Fire | [2]-BW | [3]-PurpleRain | [4]-Forest }";
         input_content->clear();
         return true;
       }
       if (*input_content == "output")
       {
         *cached_devices = service.listOutputDevices();
         *status_msg = buildDeviceListPrompt(*cached_devices);
         if (!cached_devices->empty()) {
           *mode = InputMode::OutputDevice;
         }
         input_content->clear();
         return true;
       }
       if (*input_content == "visuals")
       {
         *mode = InputMode::VisualsConfig;
         *status_msg = "Select visual: [1]-Spectrum [2]-Oscilloscope [3]-Mirrored [4]-Rolling [5]-Wave";
         input_content->clear();
         return true;
       }
        if (*input_content == CMD_PLAY) {
          service.resumePlayback();
        } else if (*input_content == CMD_STOP) {
          service.stopPlayback();
        } else if (*input_content == CMD_NEXT)
        {
          service.nextSong();
        } else if (*input_content == CMD_PREV)
        {
          service.prevSong();
        }

        // Default Command processing
        *status_msg = ProcessCommand(*input_content);
     }
     else if (*mode == InputMode::RootConfig) {
       std::string error;
       if (service.setRootPath(*input_content, error)) {
         config.setRootPath(*input_content);
         *status_msg = "Root changed to: " + *input_content;
         *reload_flag = true;
       } else {
         *status_msg = "Invalid path: " + error;
       }
       *mode = InputMode::Command;
     }
      else if (*mode == InputMode::ThemeConfig) {
        const std::string& input = *input_content;
        if (input.length() == 1) {
          char c = input[0];
          if (c >= '1' && c <= '4') {
            switch (c) {
              case '1':
                Palette::setGradient(Palette::Theme::Fire);
                *status_msg = "Theme changed to: Fire";
                break;
              case '2':
                Palette::setGradient(Palette::Theme::BW);
                *status_msg = "Theme changed to: BW";
                break;
              case '3':
                Palette::setGradient(Palette::Theme::PurpleRain);
                *status_msg = "Theme changed to: PurpleRain";
                break;
              case '4':
                Palette::setGradient(Palette::Theme::Forest);
                *status_msg = "Theme changed to: Forest";
                break;
            }
            *mode = InputMode::Command;
          } else {
            *status_msg = "Invalid number. Enter [1..4] for themes.";
            *mode = InputMode::Command;
          }
        } else {
          *status_msg = "Invalid input. Enter [1..4] for themes.";
          *mode = InputMode::Command;
        }
      }
      else if (*mode == InputMode::OutputDevice) {
        // Parse numeric selection
        int selection = 0;
        try {
          selection = std::stoi(*input_content);
        } catch (...) {
          *status_msg = "Invalid input. Enter a device number.";
          *mode = InputMode::Command;
          input_content->clear();
          return true;
        }

        if (selection >= 1 && selection <= static_cast<int>(cached_devices->size())) {
          int deviceIndex = selection - 1;
          service.setOutputDevice(deviceIndex);
          *status_msg = "Output: " + truncateDeviceName((*cached_devices)[deviceIndex]);
        } else {
          *status_msg = "Invalid selection. Enter [1.." + std::to_string(cached_devices->size()) + "].";
        }
        *mode = InputMode::Command;
      }
      else if (*mode == InputMode::VisualsConfig) {
        const std::string& input = *input_content;
        if (input.length() == 1 && input[0] >= '1' && input[0] <= '5') {
          *visualIndex = input[0] - '1';
          const char* names[] = {"Spectrum", "Oscilloscope", "Mirrored", "Rolling", "Wave"};
          *status_msg = std::string("Visual: ") + names[*visualIndex];
        } else {
          *status_msg = "Invalid. Enter [1..5]: Spectrum, Oscilloscope, Mirrored, Rolling, Wave";
        }
        *mode = InputMode::Command;
      }
     input_content->clear();
     return true;
    }
    return false;
  });

  return Renderer(component, [component, status_msg] {
    return vbox({
      text(*status_msg) | dim,
      component->Render() | border,
    });
  });
}
