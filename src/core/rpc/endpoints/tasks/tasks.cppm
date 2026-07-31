module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.tasks.tasks;

import std;

export namespace core::rpc::endpoints::tasks {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::tasks
