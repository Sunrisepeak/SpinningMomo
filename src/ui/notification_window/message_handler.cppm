module;

#include "vendor/windows.hpp"

export module sm.ui.notification_window.message_handler;

import std;

export namespace ui::notification_window::message_handler {

LRESULT CALLBACK static_window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

}  // namespace ui::notification_window::message_handler
