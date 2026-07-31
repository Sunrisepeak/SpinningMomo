module;

#include "core/state/app_state.hpp"
#include "utils/logger/logger.hpp"

module core.rpc.registry;

import std;
import core.rpc.endpoints.backup.backup;
import core.rpc.endpoints.clipboard.clipboard;
import core.rpc.endpoints.dialog.dialog;
import core.rpc.endpoints.extensions.extensions;
import core.rpc.endpoints.file.file;
import core.rpc.endpoints.gallery.gallery;
import core.rpc.endpoints.registry.registry;
import core.rpc.endpoints.runtime_info.runtime_info;
import core.rpc.endpoints.settings.settings;
import core.rpc.endpoints.tasks.tasks;
import core.rpc.endpoints.update.update;
import core.rpc.endpoints.webview.webview;
import core.rpc.endpoints.window_control.window_control;

namespace core::rpc::registry {

// 注册所有RPC端点
auto register_all_endpoints(core::AppState& state) -> void {
  Logger().info("Starting RPC endpoints registration...");

  // 注册文件操作端点
  endpoints::file::register_all(state);

  // 注册数据备份与恢复端点
  endpoints::backup::register_all(state);

  // 注册剪贴板端点
  endpoints::clipboard::register_all(state);

  // 注册应用运行时信息端点
  endpoints::runtime_info::register_all(state);

  // 注册设置端点
  endpoints::settings::register_all(state);

  // 注册后台任务端点
  endpoints::tasks::register_all(state);

  // 注册功能注册表端点
  endpoints::registry::register_all(state);

  // 注册对话框端点
  endpoints::dialog::register_all(state);

  // 注册更新端点
  endpoints::update::register_all(state);

  // 注册Webview端点
  endpoints::webview::register_all(state);

  // 注册Gallery端点
  endpoints::gallery::register_all(state);

  // 注册拓展端点
  endpoints::extensions::register_all(state);

  // 注册窗口控制端点
  endpoints::window_control::register_all(state);

  Logger().info("RPC endpoints registration completed");
}

}  // namespace core::rpc::registry
