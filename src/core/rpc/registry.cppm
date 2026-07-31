module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.registry;

import std;

export namespace core::rpc::registry {

// 注册所有RPC端点
auto register_all_endpoints(core::AppState& state) -> void;

}  // namespace core::rpc::registry
