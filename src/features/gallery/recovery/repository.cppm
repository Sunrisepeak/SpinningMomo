export module sm.features.gallery.recovery.repository;

import std;
import sm.core.state.app_state;
import sm.features.gallery.recovery.types;

export namespace features::gallery::recovery::repository {

auto get_state_by_root_path(core::AppState& app_state, const std::string& root_path)
    -> std::expected<std::optional<WatchRootRecoveryState>, std::string>;

auto upsert_state(core::AppState& app_state, const WatchRootRecoveryState& state)
    -> std::expected<void, std::string>;

auto delete_state_by_root_path(core::AppState& app_state, const std::string& root_path)
    -> std::expected<void, std::string>;

}  // namespace features::gallery::recovery::repository
