module;

#include "vendor/windows.hpp"

export module sm.features.screenshot.screenshot;

import std;
import sm.core.state.app_state;
import sm.utils.image.image;

export namespace features::screenshot {

// 主要API：异步截图
// output_dir_override: 指定时使用该目录，否则使用 output_dir_path 或 Videos/SpinningMomo
auto take_screenshot(
    core::AppState& state, HWND target_window,
    std::move_only_function<void(bool success, const std::wstring& path)> completion_callback =
        nullptr,
    utils::image::ImageFormat format = utils::image::ImageFormat::PNG, float jpeg_quality = 1.0f,
    std::optional<std::filesystem::path> output_dir_override = std::nullopt, int shutter_frames = 0,
    bool capture_client_area = true) -> std::expected<void, std::string>;

// 系统管理函数
auto cleanup_system(core::AppState& state) -> void;

}  // namespace features::screenshot
