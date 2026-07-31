module;

#include "core/state/app_state.hpp"

export module sm.core.migration.scripts.scripts;

import std;

export namespace core::migration::scripts {

// MigrationScript 定义
struct MigrationScript {
  std::string target_version;
  std::string description;
  bool run_on_fresh_install = true;
  std::function<std::expected<void, std::string>(core::AppState&)> migration_fn;
};

// 获取所有迁移脚本
auto get_all_migrations() -> const std::vector<MigrationScript>&;

}  // namespace core::migration::scripts
