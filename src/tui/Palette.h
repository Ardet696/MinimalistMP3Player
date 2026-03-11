#pragma once
#include <ftxui/screen/color.hpp>
#include <vector>

namespace Palette {
  enum class Theme { Fire, BW, PurpleRain, Forest };

  const std::vector<ftxui::Color> gradientFire = {
    ftxui::Color::Palette256(52),
    ftxui::Color::Palette256(88),
    ftxui::Color::Palette256(124),
    ftxui::Color::Palette256(160),
    ftxui::Color::Palette256(196),
    ftxui::Color::Palette256(9),
    ftxui::Color::Palette256(202),
    ftxui::Color::Palette256(208),
    ftxui::Color::Palette256(214),
    ftxui::Color::Palette256(215),
    ftxui::Color::Palette256(179),
    ftxui::Color::Palette256(180),
  };

  const std::vector<ftxui::Color> gradientBW = {
    ftxui::Color::Palette256(232),
    ftxui::Color::Palette256(234),
    ftxui::Color::Palette256(236),
    ftxui::Color::Palette256(238),
    ftxui::Color::Palette256(240),
    ftxui::Color::Palette256(242),
    ftxui::Color::Palette256(244),
    ftxui::Color::Palette256(246),
    ftxui::Color::Palette256(248),
    ftxui::Color::Palette256(250),
    ftxui::Color::Palette256(252),
    ftxui::Color::Palette256(255),
  };

  const std::vector<ftxui::Color> gradientPurpleRain = {
    ftxui::Color::Palette256(55),
    ftxui::Color::Palette256(56),
    ftxui::Color::Palette256(57),
    ftxui::Color::Palette256(93),
    ftxui::Color::Palette256(99),
    ftxui::Color::Palette256(105),
    ftxui::Color::Palette256(129),
    ftxui::Color::Palette256(135),
    ftxui::Color::Palette256(141),
    ftxui::Color::Palette256(147),
    ftxui::Color::Palette256(153),
    ftxui::Color::Palette256(171),
  };

  const std::vector<ftxui::Color> gradientForest = {
    ftxui::Color::Palette256(22),
    ftxui::Color::Palette256(28),
    ftxui::Color::Palette256(34),
    ftxui::Color::Palette256(70),
    ftxui::Color::Palette256(76),
    ftxui::Color::Palette256(82),
    ftxui::Color::Palette256(118),
    ftxui::Color::Palette256(154),
    ftxui::Color::Palette256(190),
    ftxui::Color::Palette256(191),
    ftxui::Color::Palette256(192),
    ftxui::Color::Palette256(193),
  };

  inline const std::vector<ftxui::Color>* currentGradient = &gradientFire;

  inline const std::vector<ftxui::Color>& getCurrentGradient() {
    return *currentGradient;
  }

  inline void setGradient(Theme theme) {
    switch (theme) {
      case Theme::Fire:        currentGradient = &gradientFire; break;
      case Theme::BW:         currentGradient = &gradientBW; break;
      case Theme::PurpleRain: currentGradient = &gradientPurpleRain; break;
      case Theme::Forest:     currentGradient = &gradientForest; break;
    }
  }
}
