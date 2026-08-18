export module sm.ui.shared_render_resources.shared_render_resources;

import std;
import sm.core.state.app_state;

export namespace ui::shared_render_resources {

auto ensure_initialized(core::AppState& state) -> bool;
auto cleanup(core::AppState& state) -> void;

}  // namespace ui::shared_render_resources
