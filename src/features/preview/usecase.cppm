export module sm.features.preview.usecase;

import std;
import sm.core.state.app_state;
import sm.ui.floating_window.events;

export namespace features::preview {

// 切换预览功能
auto toggle_preview(core::AppState& state) -> void;

}  // namespace features::preview
