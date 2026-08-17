module;

#include "vendor/windows.hpp"

module sm.core.commands.registry;

import std;
import sm.core.commands.state;
import sm.core.commands.types;
import sm.core.state.app_state;
import sm.features.letterbox.state;
import sm.features.letterbox.usecase;
import sm.features.overlay.state;
import sm.features.overlay.usecase;
import sm.features.photography.state;
import sm.features.photography.usecase;
import sm.features.preview.state;
import sm.features.preview.usecase;
import sm.features.recording.state;
import sm.features.recording.usecase;
import sm.features.screenshot.usecase;
import sm.features.window_control.usecase;
import sm.ui.floating_window.floating_window;
import sm.ui.webview_window.webview_window;
import sm.features.settings.state;
import sm.utils.logger.logger;
import sm.utils.path.path;
import sm.utils.system.system;

namespace core::commands {

// 注册单个内置命令：拒绝重复 ID，并把行为所有权移入注册表
auto register_command(CommandRegistry& registry, CommandDescriptor descriptor) -> void {
  const std::string id = descriptor.id;

  if (registry.descriptors.contains(id)) {
    Logger().warn("Command already registered: {}", id);
    return;
  }

  // 注册表独占 action/get_state，registration_order 只保存稳定 ID
  registry.descriptors.emplace(id, std::move(descriptor));
  registry.registration_order.push_back(id);

  Logger().debug("Registered command: {}", id);
}

// 注册所有内置命令
auto register_builtin_commands(core::AppState& state) -> void {
  auto& registry = state.commands->registry;
  Logger().info("Registering builtin commands...");

  // === 应用层命令 ===

  // 打开主界面（WebView2 或浏览器）
  register_command(registry,
                   {
                       .id = "app.main",
                       .i18n_key = "menu.app_main",
                       .is_toggle = false,
                       .action = [&state]() { ui::webview_window::activate_window(state); },
                   });

  // 退出应用
  register_command(registry, {
                                 .id = "app.exit",
                                 .i18n_key = "menu.app_exit",
                                 .is_toggle = false,
                                 .action = []() { PostQuitMessage(0); },
                             });

  // === 悬浮窗控制 ===

  // 激活悬浮窗
  register_command(registry,
                   {
                       .id = "app.float",
                       .i18n_key = "menu.app_float",
                       .is_toggle = false,
                       .action = [&state]() { ui::floating_window::toggle_visibility(state); },
                       .hotkey =
                           HotkeyBinding{
                               .modifiers = 1,  // MOD_CONTROL
                               .key = 192,      // VK_OEM_3 (`)
                               .settings_path = "app.hotkey.floating_window",
                           },
                   });

  // === 截图功能 ===

  // 截图
  register_command(registry, {
                                 .id = "screenshot.capture",
                                 .i18n_key = "menu.screenshot_capture",
                                 .is_toggle = false,
                                 .action = [&state]() { features::screenshot::capture(state); },
                                 .hotkey =
                                     HotkeyBinding{
                                         .modifiers = 0,  // 无修饰键
                                         .key = 44,       // VK_SNAPSHOT (PrintScreen)
                                         .settings_path = "app.hotkey.screenshot",
                                     },
                             });

  // 打开输出目录
  register_command(
      registry,
      {
          .id = "output.open_folder",
          .i18n_key = "menu.output_open_folder",
          .is_toggle = false,
          .action =
              [&state]() {
                auto output_dir_result =
                    utils::path::GetOutputDirectory(state.settings->raw.features.output_dir_path);
                if (!output_dir_result) {
                  Logger().error("Failed to resolve output directory: {}",
                                 output_dir_result.error());
                  return;
                }

                auto open_result = utils::system::open_directory(output_dir_result.value());
                if (!open_result) {
                  Logger().error("Failed to open output directory: {}", open_result.error());
                }
              },
      });

  // 打开游戏相册目录
  register_command(
      registry,
      {
          .id = "external_album.open_folder",
          .i18n_key = "menu.external_album_open_folder",
          .is_toggle = false,
          .action =
              [&state]() {
                std::filesystem::path folder_to_open;

                const auto& external_album_path = state.settings->raw.features.external_album_path;
                if (!external_album_path.empty()) {
                  folder_to_open = external_album_path;
                } else {
                  auto output_dir_result =
                      utils::path::GetOutputDirectory(state.settings->raw.features.output_dir_path);
                  if (!output_dir_result) {
                    Logger().error("Failed to resolve fallback output directory: {}",
                                   output_dir_result.error());
                    return;
                  }
                  folder_to_open = output_dir_result.value();
                }

                auto open_result = utils::system::open_directory(folder_to_open);
                if (!open_result) {
                  Logger().error("Failed to open external album directory: {}",
                                 open_result.error());
                }
              },
      });

  // === 独立功能 ===

  // 切换预览窗
  register_command(
      registry, {
                    .id = "preview.toggle",
                    .i18n_key = "menu.preview_toggle",
                    .is_toggle = true,
                    .action =
                        [&state]() {
                          features::preview::toggle_preview(state);
                          ui::floating_window::request_repaint(state);
                        },
                    .get_state = [&state]() -> bool {
                      return state.preview ? state.preview->running.load(std::memory_order_acquire)
                                           : false;
                    },
                });

  // 切换叠加层
  register_command(registry, {
                                 .id = "overlay.toggle",
                                 .i18n_key = "menu.overlay_toggle",
                                 .is_toggle = true,
                                 .action =
                                     [&state]() {
                                       features::overlay::toggle_overlay(state);
                                       ui::floating_window::request_repaint(state);
                                     },
                                 .get_state = [&state]() -> bool {
                                   return state.overlay && state.overlay->enabled;
                                 },
                             });

  // 切换高级摄影
  register_command(registry,
                   {
                       .id = "photography.toggle",
                       .i18n_key = "menu.photography_toggle",
                       .is_toggle = true,
                       .action =
                           [&state]() {
                             features::photography::toggle(state);
                             ui::floating_window::request_repaint(state);
                           },
                       .get_state = [&state]() -> bool { return state.photography->enabled; },
                   });

  // 切换黑边模式
  register_command(registry, {
                                 .id = "letterbox.toggle",
                                 .i18n_key = "menu.letterbox_toggle",
                                 .is_toggle = true,
                                 .action =
                                     [&state]() {
                                       features::letterbox::toggle_letterbox(state);
                                       ui::floating_window::request_repaint(state);
                                     },
                                 .get_state = [&state]() -> bool {
                                   return state.letterbox && state.letterbox->enabled;
                                 },
                             });

  // 切换录制
  register_command(
      registry, {
                    .id = "recording.toggle",
                    .i18n_key = "menu.recording_toggle",
                    .is_toggle = true,
                    .action =
                        [&state]() {
                          if (auto result = features::recording::toggle_recording(state); !result) {
                            Logger().error("Recording toggle failed: {}", result.error());
                          }
                          ui::floating_window::request_repaint(state);
                        },
                    .get_state = [&state]() -> bool {
                      if (!state.recording) {
                        return false;
                      }

                      const auto status = state.recording->status.load(std::memory_order_acquire);
                      return status == features::recording::RecordingStatus::Starting ||
                             status == features::recording::RecordingStatus::Recording ||
                             status == features::recording::RecordingStatus::Stopping;
                    },
                    .hotkey =
                        HotkeyBinding{
                            .modifiers = 0,  // 无修饰键
                            .key = 0x77,     // VK_F8 (F8)
                            .settings_path = "app.hotkey.recording",
                        },
                });

  // === 窗口操作 ===

  // 重置窗口变换
  register_command(
      registry,
      {
          .id = "window.reset",
          .i18n_key = "menu.window_reset",
          .is_toggle = false,
          .action = [&state]() { features::window_control::reset_window_transform(state); },
      });

  Logger().info("Registered {} builtin commands", registry.descriptors.size());
}

}  // namespace core::commands
