module;

#include "vendor/windows/shellapi.hpp"

export module sm.ui.tray_icon.state;

import std;

export namespace ui::tray_icon {

struct TrayIconState {
  NOTIFYICONDATAW nid{};
  bool is_created = false;
};

}  // namespace ui::tray_icon
