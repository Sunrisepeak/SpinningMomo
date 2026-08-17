module sm.core.events.registrar;

import std;
import sm.core.events.handlers.feature_handlers;
import sm.core.events.handlers.settings_handlers;
import sm.core.events.handlers.system_handlers;
import sm.core.state.app_state;

namespace core::events {

auto register_all_handlers(core::AppState& app_state) -> void {
  handlers::register_feature_handlers(app_state);
  handlers::register_settings_handlers(app_state);
  handlers::register_system_handlers(app_state);
}

}  // namespace core::events
