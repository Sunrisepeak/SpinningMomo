export module sm.features.gallery.color.filter;

import std;
import sm.core.database.types;
import sm.features.gallery.types;

export namespace features::gallery::color::filter {

auto append_color_filter_conditions(const features::gallery::QueryAssetsFilters& filters,
                                    std::vector<std::string>& conditions,
                                    std::vector<core::database::DbParam>& params,
                                    std::string_view asset_table_alias = "")
    -> std::expected<void, std::string>;

}  // namespace features::gallery::color::filter
