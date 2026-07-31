module;

#include "core/state/app_state.hpp"

export module core.initializer.initializer;

import std;

export namespace core::initializer {

auto initialize_application(core::AppState& state) -> std::expected<void, std::string>;

}
