export module sm.features.settings.compute;

import std;
import sm.core.state.app_state;

export namespace features::settings::compute {

// 更新状态的计算部分
// 触发计算状态更新 (Reactivity Trigger)
auto trigger_compute(core::AppState& app_state) -> bool;

}  // namespace features::settings::compute
