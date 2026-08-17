export module sm.core.shutdown.shutdown;

import std;
import sm.core.state.app_state;

export namespace core::shutdown {

auto shutdown_application(core::AppState& state) -> void;

}
