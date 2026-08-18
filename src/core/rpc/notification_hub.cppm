export module sm.core.rpc.notification_hub;

import std;
import sm.core.state.app_state;

export namespace core::rpc::notification_hub {

// 同时分发到 WebView 和 SSE 的统一通知出口
auto send_notification(core::AppState& state, const std::string& method,
                       const std::string& params_json = "{}") -> void;

}  // namespace core::rpc::notification_hub
