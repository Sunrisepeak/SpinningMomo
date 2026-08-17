module;

#include "vendor/windows.hpp"

export module sm.ui.context_menu.render_context;

import std;
import sm.core.state.app_state;
import sm.ui.context_menu.state;

export namespace ui::context_menu::render_context {

auto initialize_text_format(core::AppState& app_state) -> bool;

// 主菜单D2D资源管理
auto initialize_context_menu(core::AppState& app_state, HWND hwnd) -> bool;
auto cleanup_context_menu(core::AppState& app_state) -> void;

// 子菜单D2D资源管理
auto initialize_submenu(core::AppState& app_state, HWND hwnd) -> bool;
auto cleanup_submenu(core::AppState& app_state) -> void;

// 调整渲染目标大小
auto resize_context_menu(core::AppState& app_state, const SIZE& new_size) -> bool;
auto resize_submenu(core::AppState& app_state, const SIZE& new_size) -> bool;

}  // namespace ui::context_menu::render_context
