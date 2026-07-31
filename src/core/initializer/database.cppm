module;

#include "core/database/state.hpp"
#include "core/state/app_state.hpp"

export module core.initializer.database;

import std;

export namespace core::initializer::database {

// 初始化数据库
auto initialize_database(core::AppState& state) -> std::expected<void, std::string>;

}  // namespace core::initializer::database
