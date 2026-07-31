module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.window_control.window_control;

import std;

export namespace core::rpc::endpoints::window_control {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::window_control
