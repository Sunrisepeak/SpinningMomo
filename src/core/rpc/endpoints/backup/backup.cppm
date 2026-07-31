module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.backup.backup;

import std;

export namespace core::rpc::endpoints::backup {

// 注册数据导出和完全替换恢复端点。
auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::backup
