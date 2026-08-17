module;

#include "vendor/windows.hpp"
#include "vendor/windows/d2d1_3.hpp"

export module sm.ui.shared_theme.shared_theme;

import std;
import sm.core.state.app_state;

export namespace ui::shared_theme {

struct FloatingWindowThemeColors {
  D2D1_COLOR_F background{};
  D2D1_COLOR_F separator{};
  D2D1_COLOR_F text{};
  D2D1_COLOR_F indicator{};
  D2D1_COLOR_F hover{};
  D2D1_COLOR_F title_bar{};
  D2D1_COLOR_F scroll_indicator{};
};

auto resolve_floating_window_theme_colors(const core::AppState& state) -> FloatingWindowThemeColors;

}  // namespace ui::shared_theme
