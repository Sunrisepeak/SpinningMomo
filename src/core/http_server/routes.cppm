export module sm.core.http_server.routes;

import std;
import sm.core.state.app_state;
import sm.vendor.uwebsockets;

export namespace core::http_server::routes {
// 注册所有路由
auto register_routes(core::AppState& state, uWS::App& app) -> void;
}  // namespace core::http_server::routes
