module;

#include "core/state/app_state.hpp"

export module sm.core.events.handlers.feature_handlers;

import std;

export namespace core::events::handlers {

auto register_feature_handlers(core::AppState& app_state) -> void;

}
