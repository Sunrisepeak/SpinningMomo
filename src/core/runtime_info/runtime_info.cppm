module;

#include "core/state/app_state.hpp"

export module core.runtime_info.runtime_info;

import std;

export namespace core::runtime_info {

// 采集运行时信息并写入 state.runtime_info，同时输出关键日志
auto collect(core::AppState& app_state) -> void;

}  // namespace core::runtime_info
