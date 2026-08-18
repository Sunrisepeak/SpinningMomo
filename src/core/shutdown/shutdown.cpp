module sm.core.shutdown.shutdown;

import std;
import sm.core.commands.registry;
import sm.core.dialog_service.dialog_service;
import sm.core.http_server.http_server;
import sm.core.state.app_state;
import sm.core.worker_pool.worker_pool;
import sm.extensions.infinity_nikki.photo_service;
import sm.features.gallery.gallery;
import sm.features.letterbox.letterbox;
import sm.features.overlay.overlay;
import sm.features.photography.usecase;
import sm.features.preview.preview;
import sm.features.recording.usecase;
import sm.features.screenshot.screenshot;
import sm.features.update.state;
import sm.features.window_control.window_control;
import sm.ui.context_menu.context_menu;
import sm.ui.floating_window.floating_window;
import sm.ui.floating_window.state;
import sm.ui.notification_window.notification_window;
import sm.ui.photography_panel.photography_panel;
import sm.ui.tray_icon.tray_icon;
import sm.ui.webview_window.webview_window;

import sm.utils.logger.logger;

import sm.core.async.async;
import sm.core.database.database;
import sm.core.http_client.http_client;
import sm.features.update.update;

namespace core::shutdown {

// 按初始化的反向依赖关闭应用，先收敛后台任务再释放 UI 和核心服务。
auto shutdown_application(core::AppState& state) -> void {
  Logger().info("==================================================");
  Logger().info("SpinningMomo shutdown begin");
  Logger().info("==================================================");

  // 先卸载键盘、鼠标钩子，避免后续清理过程输入事件被拦截
  core::commands::uninstall_keyboard_keepalive_hook(state);

  core::commands::unregister_all_hotkeys(state, state.floating_window->window.hwnd);

  features::window_control::stop_center_lock_monitor(state);

  // 清理顺序应该与 core::initializer::initialize_application 中的初始化顺序相反
  // 先停止录制并等待录制切换线程结束，避免与后续 UI/核心清理并发
  features::recording::stop_recording_if_running(state);

  core::dialog_service::stop(state);

  auto shutdown_gallery_extensions = [](core::AppState& app_state) {
    extensions::infinity_nikki::photo_service::shutdown(app_state);
  };
  features::gallery::cleanup(state, std::move(shutdown_gallery_extensions));

  // 1. UI 清理
  ui::context_menu::cleanup(state);
  ui::tray_icon::destroy(state);
  ui::notification_window::cleanup(state);
  ui::photography_panel::cleanup(state);
  ui::floating_window::destroy_window(state);
  ui::webview_window::cleanup(state);

  // 2. 功能模块清理
  // 检查是否有待处理的更新
  if (state.update->pending_update) {
    Logger().info("Executing pending update on program exit");
    features::update::execute_pending_update(state);
  }
  features::preview::stop_preview(state);
  features::preview::cleanup_preview(state);
  features::photography::stop(state);
  features::photography::cleanup(state);
  features::overlay::stop_overlay(state);
  features::overlay::cleanup_overlay(state);
  if (auto result = features::letterbox::shutdown(state); !result) {
    Logger().error("Failed to shutdown Letterbox: {}", result.error());
  }
  features::screenshot::cleanup_system(state);
  // 3. 核心服务清理
  core::http_server::shutdown(state);
  core::http_client::shutdown(state);

  // 停止工作线程池（等待所有任务完成）
  core::worker_pool::stop(state);

  core::database::shutdown(state);

  core::async::stop(state);

  Logger().info("==================================================");
  Logger().info("SpinningMomo shutdown complete");
  Logger().info("==================================================");
}

}  // namespace core::shutdown
