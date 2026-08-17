module;

#include "vendor/windows.hpp"

export module sm.ui.photography_panel.painter;

import std;
import sm.core.state.app_state;
import sm.ui.photography_panel.state;

export namespace ui::photography_panel::painter {

auto compute_panel_layout(const core::AppState& state) -> ui::photography_panel::PanelLayoutMetrics;
auto shutter_to_x(const RECT& rect, int frames) -> float;
auto paint(core::AppState& state, HWND hwnd) -> void;

}  // namespace ui::photography_panel::painter
