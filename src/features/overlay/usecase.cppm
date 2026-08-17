export module sm.features.overlay.usecase;

import std;
import sm.core.state.app_state;
import sm.ui.floating_window.events;

export namespace features::overlay {

// 切换叠加层功能
auto toggle_overlay(core::AppState& state) -> void;

}  // namespace features::overlay
