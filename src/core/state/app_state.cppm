export module sm.core.state.app_state;

import std;

// AppState is the project's single root: it owns one std::unique_ptr per
// subsystem state. As a header it FORWARD-DECLARED all of them to avoid
// including the world.
//
// A forward declaration cannot survive modularisation. `struct HttpServerState;`
// written here is attached to THIS module, while the definition is attached to
// `sm.core.http_server.state` — two different entities, and clang says so:
//
//   error: declaration 'HttpServerState' attached to named module
//   'sm.core.http_server.state' cannot be attached to other modules
//
// So the declarations become imports. Nothing is lost: the dependency was
// always there, it was just unwritten. The graph stays acyclic because a state
// type never needs AppState — checked, and the architecture guard keeps it that
// way.
import sm.core.async.state;
import sm.core.commands.state;
import sm.core.database.state;
import sm.core.dialog_service.state;
import sm.core.events.state;
import sm.core.http_client.state;
import sm.core.http_server.state;
import sm.core.i18n.state;
import sm.core.rpc.state;
import sm.core.state.runtime_info;
import sm.core.tasks.state;
import sm.core.webview.state;
import sm.core.worker_pool.state;
import sm.features.gallery.state;
import sm.features.letterbox.state;
import sm.features.overlay.state;
import sm.features.photography.state;
import sm.features.preview.state;
import sm.features.recording.state;
import sm.features.screenshot.state;
import sm.features.settings.state;
import sm.features.update.state;
import sm.features.window_control.state;
import sm.ui.context_menu.state;
import sm.ui.floating_window.state;
import sm.ui.notification_window.state;
import sm.ui.photography_panel.state;
import sm.ui.shared_render_resources.state;
import sm.ui.tray_icon.state;

namespace core {

struct AppState {
  AppState();
  ~AppState();

  // 应用级状态
  std::unique_ptr<core::rpc::RpcState> rpc;
  std::unique_ptr<core::async::AsyncState> async;
  std::unique_ptr<core::dialog_service::DialogServiceState> dialog_service;
  std::unique_ptr<core::events::EventsState> events;
  std::unique_ptr<core::i18n::I18nState> i18n;
  std::unique_ptr<core::webview::WebViewState> webview;
  std::unique_ptr<core::runtime_info::RuntimeInfoState> runtime_info;
  std::unique_ptr<core::database::DatabaseState> database;
  std::unique_ptr<core::http_server::HttpServerState> http_server;
  std::unique_ptr<core::http_client::HttpClientState> http_client;
  std::unique_ptr<core::worker_pool::WorkerPoolState> worker_pool;
  std::unique_ptr<core::commands::CommandState> commands;
  std::unique_ptr<core::tasks::TaskState> tasks;

  // 应用设置状态（包含配置和计算状态）
  std::unique_ptr<features::settings::SettingsState> settings;

  // 更新模块状态
  std::unique_ptr<features::update::UpdateState> update;

  // UI状态
  std::unique_ptr<ui::shared_render_resources::SharedRenderResourcesState> shared_render_resources;
  std::unique_ptr<ui::floating_window::FloatingWindowState> floating_window;
  std::unique_ptr<ui::tray_icon::TrayIconState> tray_icon;
  std::unique_ptr<ui::context_menu::ContextMenuState> context_menu;
  std::unique_ptr<ui::notification_window::NotificationWindowState> notification_window;
  std::unique_ptr<ui::photography_panel::PhotographyPanelState> photography_panel;

  // 功能模块状态
  std::unique_ptr<features::letterbox::LetterboxState> letterbox;
  std::unique_ptr<features::gallery::GalleryState> gallery;
  std::unique_ptr<features::overlay::OverlayState> overlay;
  std::unique_ptr<features::preview::PreviewState> preview;
  std::unique_ptr<features::window_control::WindowControlState> window_control;
  std::unique_ptr<features::screenshot::ScreenshotState> screenshot;
  std::unique_ptr<features::recording::RecordingState> recording;
  std::unique_ptr<features::photography::PhotographyState> photography;
};

}  // namespace core
