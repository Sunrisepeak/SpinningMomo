module;

#include "vendor/windows.hpp"

export module sm.ui.context_menu.context_menu;

import std;
import sm.core.state.app_state;
import sm.ui.context_menu.types;

export namespace ui::context_menu {

auto initialize(core::AppState& state) -> std::expected<void, std::string>;

auto cleanup(core::AppState& state) -> void;

auto Show(core::AppState& state, std::vector<MenuItem> items, const POINT& position) -> void;

auto hide_and_destroy_menu(core::AppState& state) -> void;

auto hide_submenu(core::AppState& state) -> void;

auto show_submenu(core::AppState& state, int index) -> void;

auto handle_menu_action(core::AppState& state, const ui::context_menu::MenuItem& item) -> void;

}  // namespace ui::context_menu
