export module sm.core.rpc.endpoints.runtime_info.runtime_info;

import std;
import sm.core.state.app_state;

export namespace core::rpc::endpoints::runtime_info {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::runtime_info
