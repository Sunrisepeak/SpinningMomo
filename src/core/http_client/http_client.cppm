export module sm.core.http_client.http_client;

import std;
import sm.core.http_client.types;
import sm.core.state.app_state;
import asio;

export namespace core::http_client {

auto initialize(core::AppState& state) -> std::expected<void, std::string>;

auto shutdown(core::AppState& state) -> void;

auto fetch(core::AppState& state, const core::http_client::Request& request)
    -> asio::awaitable<std::expected<core::http_client::Response, std::string>>;

auto download_to_file(core::AppState& state, const core::http_client::Request& request,
                      const std::filesystem::path& output_path,
                      core::http_client::DownloadProgressCallback progress_callback = nullptr)
    -> asio::awaitable<std::expected<void, std::string>>;

}  // namespace core::http_client
