export module sm.core.rpc.endpoints.backup.backup;

import std;
import sm.core.state.app_state;

export namespace core::rpc::endpoints::backup {

// 注册数据导出和完全替换恢复端点。
auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::backup
