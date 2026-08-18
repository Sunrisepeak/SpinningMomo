export module sm.ui.tray_icon.tray_icon;

import std;
import sm.core.state.app_state;

export namespace ui::tray_icon {

auto create(core::AppState& state) -> std::expected<void, std::string>;

auto destroy(core::AppState& state) -> void;

auto show_context_menu(core::AppState& state) -> void;

}  // namespace ui::tray_icon
