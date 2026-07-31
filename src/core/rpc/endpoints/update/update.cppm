module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.update.update;

import std;

export namespace core::rpc::endpoints::update {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::update
