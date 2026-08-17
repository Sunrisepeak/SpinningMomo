module sm.features.overlay.usecase;

import std;
import sm.core.i18n.state;
import sm.core.notifications.notifications;
import sm.core.state.app_state;
import sm.features.letterbox.letterbox;
import sm.features.letterbox.state;
import sm.features.overlay.overlay;
import sm.features.overlay.state;
import sm.features.preview.preview;
import sm.features.preview.state;
import sm.features.window_control.window_control;

import sm.features.settings.state;
import sm.utils.logger.logger;
import sm.utils.string.string;

namespace features::overlay {

// 切换叠加层功能
auto toggle_overlay(core::AppState& state) -> void {
  bool is_enabled = state.overlay->enabled;

  // 切换启用状态
  state.overlay->enabled = !is_enabled;

  if (!is_enabled) {
    // 用户想启用叠加层
    // 预览窗与叠加层互斥，若预览窗运行则先关闭
    if (state.preview && state.preview->running.load(std::memory_order_acquire)) {
      features::preview::stop_preview(state);
      core::notifications::show_notification(state, state.i18n->texts["label.app_name"],
                                             state.i18n->texts["message.preview_overlay_conflict"]);
    }
    // 如果启用了黑边模式，关闭黑边窗口
    if (state.letterbox->enabled) {
      if (auto result = features::letterbox::shutdown(state); !result) {
        Logger().error("Failed to shutdown letterbox: {}", result.error());
      }
    }

    std::wstring window_title = utils::string::FromUtf8(state.settings->raw.window.target_title);
    auto target_window = features::window_control::find_target_window(window_title);

    if (target_window) {
      if (auto result = features::overlay::start_overlay(state, target_window.value()); !result) {
        Logger().error("Failed to start overlay: {}", result.error());
        // 回滚启用状态
        state.overlay->enabled = false;
        // 使用新的消息定义并附加错误详情
        std::string error_message =
            state.i18n->texts["message.overlay_start_failed"] + result.error();
        core::notifications::show_notification(state, state.i18n->texts["label.app_name"],
                                               error_message);
      }
    } else {
      // 找不到目标窗口
      Logger().warn("No target window found for overlay");
      state.overlay->enabled = false;
      core::notifications::show_notification(state, state.i18n->texts["label.app_name"],
                                             state.i18n->texts["message.window_not_found"]);
    }
  } else {
    // 用户想停用叠加层
    features::overlay::stop_overlay(state);

    // 如果启用了黑边模式，重新显示黑边窗口
    if (state.letterbox->enabled) {
      std::wstring window_title = utils::string::FromUtf8(state.settings->raw.window.target_title);
      auto target_window = features::window_control::find_target_window(window_title);
      if (target_window) {
        if (auto result = features::letterbox::show(state, target_window.value()); !result) {
          Logger().error("Failed to show letterbox: {}", result.error());
        }
      }
    }
  }
}

}  // namespace features::overlay
