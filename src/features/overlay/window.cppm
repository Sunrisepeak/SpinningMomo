module;

#include "vendor/windows.hpp"

export module sm.features.overlay.window;

import std;
import sm.core.state.app_state;
import sm.features.overlay.state;

export namespace features::overlay::window {

// 创建叠加层窗口
auto create_overlay_window(HINSTANCE instance, core::AppState& state)
    -> std::expected<HWND, std::string>;

// 初始化叠加层窗口系统
auto initialize_overlay_window(core::AppState& state, HINSTANCE instance)
    -> std::expected<void, std::string>;

// 显示叠加层窗口（首次显示）
auto show_overlay_window_first_time(core::AppState& state) -> std::expected<void, std::string>;

// 隐藏叠加层窗口
auto hide_overlay_window(core::AppState& state) -> void;

// 更新叠加层窗口尺寸
auto set_overlay_window_size(core::AppState& state, int game_width, int game_height) -> void;

// 销毁叠加层窗口
auto destroy_overlay_window(core::AppState& state) -> void;

// 注销叠加层窗口类
auto unregister_overlay_window_class(HINSTANCE instance) -> void;

// 恢复游戏窗口
auto restore_game_window(core::AppState& state) -> void;

}  // namespace features::overlay::window
