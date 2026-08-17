export module sm.extensions.infinity_nikki.metadata_dict;

import std;
import sm.core.state.app_state;
import sm.extensions.infinity_nikki.types;
import asio;

export namespace extensions::infinity_nikki::metadata_dict {

auto resolve_metadata_names(core::AppState& app_state,
                            const GetInfinityNikkiMetadataNamesParams& params)
    -> asio::awaitable<std::expected<InfinityNikkiMetadataNames, std::string>>;

}  // namespace extensions::infinity_nikki::metadata_dict
