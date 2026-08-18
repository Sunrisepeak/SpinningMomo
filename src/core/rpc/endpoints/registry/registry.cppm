export module sm.core.rpc.endpoints.registry.registry;

import std;
import sm.core.state.app_state;

export namespace core::rpc::endpoints::registry {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::registry
