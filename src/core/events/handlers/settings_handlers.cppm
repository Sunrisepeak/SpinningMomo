module;

#include "core/state/app_state.hpp"

export module core.events.handlers.settings_handlers;

import std;

export namespace core::events::handlers {

auto register_settings_handlers(core::AppState& app_state) -> void;

}
