module sm.features.photography.usecase;

import std;
import sm.core.i18n.state;
import sm.core.notifications.notifications;
import sm.core.state.app_state;
import sm.features.photography.state;
import sm.ui.photography_panel.photography_panel;

import sm.utils.logger.logger;

namespace features::photography {

// 显示面板并标记高级摄影启用
auto start(core::AppState& state) -> std::expected<void, std::string> {
  auto show_result = ui::photography_panel::show(state);
  if (!show_result) {
    return std::unexpected(show_result.error());
  }

  const bool previous = state.photography->enabled.exchange(true, std::memory_order_acq_rel);
  if (!previous) {
    Logger().info("Photography mode started");
  }
  return {};
}

// 停止高级摄影时关闭面板
auto stop(core::AppState& state) -> void {
  const bool previous = state.photography->enabled.exchange(false, std::memory_order_acq_rel);
  ui::photography_panel::hide(state);

  if (previous) {
    Logger().info("Photography mode stopped");
  }
}

// 已启用则停止；否则尝试启动，失败时弹通知。
auto toggle(core::AppState& state) -> void {
  if (state.photography->enabled.load(std::memory_order_acquire)) {
    stop(state);
    return;
  }

  if (auto result = start(state); !result) {
    Logger().error("Failed to start photography mode: {}", result.error());
    core::notifications::show_notification(
        state, state.i18n->texts["label.app_name"],
        state.i18n->texts["message.photography_start_failed"] + result.error());
  }
}

auto cleanup(core::AppState& state) -> void {
  state.photography->enabled.store(false, std::memory_order_release);
}

auto handle_panel_close(core::AppState& state) -> void {
  state.photography->enabled.store(false, std::memory_order_release);
}

}  // namespace features::photography
