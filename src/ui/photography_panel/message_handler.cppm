module;

#include "vendor/windows.hpp"

export module sm.ui.photography_panel.message_handler;

import std;

export namespace ui::photography_panel::message_handler {

LRESULT CALLBACK static_window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

}  // namespace ui::photography_panel::message_handler
