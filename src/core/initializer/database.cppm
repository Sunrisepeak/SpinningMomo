export module sm.core.initializer.database;

import std;
import sm.core.database.state;
import sm.core.state.app_state;

export namespace core::initializer::database {

// 初始化数据库
auto initialize_database(core::AppState& state) -> std::expected<void, std::string>;

}  // namespace core::initializer::database
