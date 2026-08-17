export module sm.core.http_server.http_server;

import std;
import sm.core.state.app_state;

export namespace core::http_server {
// 初始化HTTP服务器
auto initialize(core::AppState& state) -> std::expected<void, std::string>;

// 关闭服务器
auto shutdown(core::AppState& state) -> void;

// 获取SSE连接数量
auto get_sse_connection_count(const core::AppState& state) -> std::size_t;
}  // namespace core::http_server
