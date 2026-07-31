module;

#include "core/state/app_state.hpp"

export module sm.core.events.registrar;

import std;

export namespace core::events {

auto register_all_handlers(core::AppState& app_state) -> void;

}  // namespace core::events
