export module sm.features.screenshot.usecase;

import std;
import sm.core.state.app_state;
import sm.ui.floating_window.events;

export namespace features::screenshot {

// 截图（推荐使用）
auto capture(core::AppState& state) -> void;

// 处理截图事件（Event版本，用于热键系统）
auto handle_capture_event(core::AppState& state,
                          const ui::floating_window::events::CaptureEvent& event) -> void;

}  // namespace features::screenshot
