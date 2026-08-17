export module sm.core.events.registrar;

import std;
import sm.core.state.app_state;

export namespace core::events {

auto register_all_handlers(core::AppState& app_state) -> void;

}  // namespace core::events
