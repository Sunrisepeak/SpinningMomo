module sm.core.state.app_state;

import std;
import sm.core.commands.state;
import sm.core.database.state;
import sm.core.dialog_service.state;
import sm.core.events.state;
import sm.core.i18n.state;
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
import sm.features.update.state;
import sm.features.window_control.state;
import sm.ui.context_menu.state;
import sm.ui.floating_window.state;
import sm.ui.notification_window.state;
import sm.ui.photography_panel.state;
import sm.ui.shared_render_resources.state;
import sm.ui.tray_icon.state;

import sm.features.settings.state;
import sm.core.async.state;
import sm.core.http_client.state;
import sm.core.http_server.state;
import sm.core.rpc.state;

namespace core {

AppState::AppState()
    : rpc(std::make_unique<core::rpc::RpcState>()),
      async(std::make_unique<core::async::AsyncState>()),
      dialog_service(std::make_unique<core::dialog_service::DialogServiceState>()),
      events(std::make_unique<core::events::EventsState>()),
      i18n(std::make_unique<core::i18n::I18nState>()),
      webview(std::make_unique<core::webview::WebViewState>()),
      runtime_info(std::make_unique<core::runtime_info::RuntimeInfoState>()),
      database(std::make_unique<core::database::DatabaseState>()),
      http_server(std::make_unique<core::http_server::HttpServerState>()),
      http_client(std::make_unique<core::http_client::HttpClientState>()),
      worker_pool(std::make_unique<core::worker_pool::WorkerPoolState>()),
      commands(std::make_unique<core::commands::CommandState>()),
      tasks(std::make_unique<core::tasks::TaskState>()),
      settings(std::make_unique<features::settings::SettingsState>()),
      update(std::make_unique<features::update::UpdateState>()),
      shared_render_resources(
          std::make_unique<ui::shared_render_resources::SharedRenderResourcesState>()),
      floating_window(std::make_unique<ui::floating_window::FloatingWindowState>()),
      tray_icon(std::make_unique<ui::tray_icon::TrayIconState>()),
      context_menu(std::make_unique<ui::context_menu::ContextMenuState>()),
      notification_window(std::make_unique<ui::notification_window::NotificationWindowState>()),
      photography_panel(std::make_unique<ui::photography_panel::PhotographyPanelState>()),
      letterbox(std::make_unique<features::letterbox::LetterboxState>()),
      gallery(std::make_unique<features::gallery::GalleryState>()),
      overlay(std::make_unique<features::overlay::OverlayState>()),
      preview(std::make_unique<features::preview::PreviewState>()),
      window_control(std::make_unique<features::window_control::WindowControlState>()),
      screenshot(std::make_unique<features::screenshot::ScreenshotState>()),
      recording(std::make_unique<features::recording::RecordingState>()),
      photography(std::make_unique<features::photography::PhotographyState>()) {}

AppState::~AppState() = default;

}  // namespace core
