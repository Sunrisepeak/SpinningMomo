export module sm.extensions.infinity_nikki.photo_extract.photo_extract;

import std;
import sm.core.state.app_state;
import sm.extensions.infinity_nikki.types;
import asio;

export namespace extensions::infinity_nikki::photo_extract {

auto extract_photo_params(
    core::AppState& app_state, const InfinityNikkiExtractPhotoParamsRequest& request,
    const std::function<void(const InfinityNikkiExtractPhotoParamsProgress&)>& progress_callback)
    -> asio::awaitable<std::expected<InfinityNikkiExtractPhotoParamsResult, std::string>>;

auto extract_photo_params_silent_incremental(
    core::AppState& app_state, const InfinityNikkiSilentExtractPhotoParamsRequest& request,
    const std::function<void(const InfinityNikkiExtractPhotoParamsProgress&)>& progress_callback)
    -> asio::awaitable<std::expected<InfinityNikkiExtractPhotoParamsResult, std::string>>;

}  // namespace extensions::infinity_nikki::photo_extract
