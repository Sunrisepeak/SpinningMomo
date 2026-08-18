export module sm.ui.photography_panel.photography_panel;

import std;
import sm.core.state.app_state;

export namespace ui::photography_panel {

auto show(core::AppState& state) -> std::expected<void, std::string>;
auto hide(core::AppState& state) -> void;
auto request_repaint(core::AppState& state) -> void;
auto refresh_from_settings(core::AppState& state) -> void;
auto cleanup(core::AppState& state) -> void;

}  // namespace ui::photography_panel
