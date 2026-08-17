export module sm.features.letterbox.usecase;

import std;
import sm.core.state.app_state;
import sm.ui.floating_window.events;

export namespace features::letterbox {

// 切换黑边模式
auto toggle_letterbox(core::AppState& state) -> void;

}  // namespace features::letterbox
