module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.settings.settings;

import std;

export namespace core::rpc::endpoints::settings {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::settings
