export module sm.features.gallery.color.repository;

import std;
import sm.core.state.app_state;
import sm.features.gallery.color.types;
import sm.features.gallery.types;

export namespace features::gallery::color::repository {

// 在调用方已经建立的事务中替换颜色；本函数不单独提交，供资产聚合写入复用。
auto replace_asset_colors_in_transaction(core::AppState& app_state, std::int64_t asset_id,
                                         const std::vector<ExtractedColor>& colors)
    -> std::expected<void, std::string>;

auto get_asset_main_colors(core::AppState& app_state, std::int64_t asset_id)
    -> std::expected<std::vector<features::gallery::AssetMainColor>, std::string>;

}  // namespace features::gallery::color::repository
