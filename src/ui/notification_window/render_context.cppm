module;

#include "vendor/windows.hpp"

export module sm.ui.notification_window.render_context;

import std;
import sm.core.state.app_state;

export namespace ui::notification_window::render_context {

auto ensure_render_context(core::AppState& state) -> bool;
auto cleanup_render_context(core::AppState& state) -> void;
auto resize_render_context(core::AppState& state, const SIZE& new_size) -> bool;

}  // namespace ui::notification_window::render_context
