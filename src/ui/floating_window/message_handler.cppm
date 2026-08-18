module;

#include "vendor/windows.hpp"

export module sm.ui.floating_window.message_handler;

import std;

export namespace ui::floating_window::message_handler {

LRESULT CALLBACK static_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

}  // namespace ui::floating_window::message_handler
