export module sm.features.gallery.root_availability;

import std;
import sm.core.state.app_state;
import sm.features.gallery.state;
import sm.features.gallery.types;

export namespace features::gallery::root_availability {

constexpr std::chrono::milliseconds kDefaultRemoteProbeTimeout{1200};

auto initialize(core::AppState& app_state) -> std::expected<void, std::string>;

auto get_for_root_id(core::AppState& app_state, std::int64_t root_id)
    -> std::optional<features::gallery::RootAvailability>;

auto get_for_path(core::AppState& app_state, const std::filesystem::path& root_path)
    -> features::gallery::RootAvailability;

auto is_remote_unreachable(core::AppState& app_state, std::int64_t root_id) -> bool;

auto is_remote_unreachable(core::AppState& app_state, const std::filesystem::path& root_path)
    -> bool;

auto availability_to_string(features::gallery::RootAvailability availability) -> std::string_view;

}  // namespace features::gallery::root_availability
