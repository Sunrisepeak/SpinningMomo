export module sm.extensions.infinity_nikki.map_service;

import std;
import sm.core.state.app_state;

export namespace extensions::infinity_nikki::map_service {

// 注册 Infinity Nikki 官方地图页面所需的 WebView 注入脚本。
auto register_from_settings(core::AppState& app_state) -> void;

}  // namespace extensions::infinity_nikki::map_service
