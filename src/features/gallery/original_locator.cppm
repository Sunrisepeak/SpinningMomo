export module sm.features.gallery.original_locator;

import std;
import sm.core.state.app_state;
import sm.features.gallery.types;

export namespace features::gallery::original_locator {

auto make_root_host_name(std::int64_t root_id) -> std::wstring;

auto populate_asset_locators(core::AppState& app_state, std::vector<Asset>& assets)
    -> std::expected<void, std::string>;

auto resolve_original_file_path(core::AppState& app_state, std::int64_t root_id,
                                std::string_view relative_path)
    -> std::expected<std::filesystem::path, std::string>;

}  // namespace features::gallery::original_locator
