module;

#include "core/state/app_state.hpp"

export module core.shutdown.shutdown;

import std;

export namespace core::shutdown {

auto shutdown_application(core::AppState& state) -> void;

}
