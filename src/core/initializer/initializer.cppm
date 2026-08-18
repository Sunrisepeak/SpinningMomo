export module sm.core.initializer.initializer;

import std;
import sm.core.state.app_state;

export namespace core::initializer {

auto initialize_application(core::AppState& state) -> std::expected<void, std::string>;

}
