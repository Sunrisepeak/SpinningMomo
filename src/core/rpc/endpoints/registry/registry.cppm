module;

#include "core/state/app_state.hpp"

export module core.rpc.endpoints.registry.registry;

import std;

export namespace core::rpc::endpoints::registry {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::registry
