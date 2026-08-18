export module sm.core.rpc.endpoints.settings.settings;

import std;
import sm.core.state.app_state;

export namespace core::rpc::endpoints::settings {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::settings
