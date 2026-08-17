export module sm.core.rpc.endpoints.webview.webview;

import std;
import sm.core.state.app_state;

export namespace core::rpc::endpoints::webview {

// 注册RPC方法
auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::webview
