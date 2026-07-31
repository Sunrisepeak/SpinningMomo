module;

#include "core/state/app_state.hpp"

export module core.rpc.endpoints.webview.webview;

import std;

export namespace core::rpc::endpoints::webview {

// 注册RPC方法
auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::webview
