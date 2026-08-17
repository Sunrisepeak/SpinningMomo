module;

#include "core/state/app_state.hpp"
#include "extensions/infinity_nikki/types.hpp"

export module sm.extensions.infinity_nikki.metadata_dict;

import std;
import asio;

export namespace extensions::infinity_nikki::metadata_dict {

auto resolve_metadata_names(core::AppState& app_state,
                            const GetInfinityNikkiMetadataNamesParams& params)
    -> asio::awaitable<std::expected<InfinityNikkiMetadataNames, std::string>>;

}  // namespace extensions::infinity_nikki::metadata_dict
