module;

#include "vendor/windows.hpp"

export module sm.ui.tray_icon.types;

import std;

export namespace ui::tray_icon {

// 托盘图标相关常量
constexpr UINT WM_TRAYICON = WM_USER + 1;  // WM_USER + 1
constexpr UINT HOTKEY_ID = 1;
constexpr int IDI_ICON1 = 101;
inline const std::wstring APP_NAME = L"SpinningMomo";

}  // namespace ui::tray_icon
