module;

#include "vendor/windows.hpp"
#include "vendor/windows/d2d1_3.hpp"

export module sm.ui.floating_window.painter;

import std;
import sm.core.state.app_state;
import sm.ui.floating_window.types;

export namespace ui::floating_window::painter {

// 内部函数声明
auto draw_background(const core::AppState& state, const D2D1_RECT_F& rect) -> void;
auto draw_title_bar(const core::AppState& state, const D2D1_RECT_F& rect) -> void;
auto draw_separators(const core::AppState& state, const D2D1_RECT_F& rect) -> void;
auto draw_items(core::AppState& state, const D2D1_RECT_F& rect) -> void;
auto draw_single_item(core::AppState& state, const ui::floating_window::MenuItem& item,
                      const D2D1_RECT_F& item_rect, bool is_hovered) -> void;
auto draw_scroll_indicator(const core::AppState& state, const D2D1_RECT_F& column_rect,
                           std::size_t total_items, std::size_t scroll_offset, bool is_hovered,
                           bool is_last_column) -> void;

// 主绘制函数
auto paint(core::AppState& state, HWND hwnd, const RECT& client_rect) -> void;

}  // namespace ui::floating_window::painter
