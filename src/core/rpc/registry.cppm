export module sm.core.rpc.registry;

import std;
import sm.core.state.app_state;

export namespace core::rpc::registry {

// 注册所有RPC端点
auto register_all_endpoints(core::AppState& state) -> void;

}  // namespace core::rpc::registry
