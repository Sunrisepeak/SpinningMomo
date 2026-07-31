module;

#include "core/state/app_state.hpp"
#include "vendor/asio.hpp"
#include "core/rpc/types.hpp"

module core.rpc.endpoints.clipboard.clipboard;

import std;
import core.rpc.rpc;
import core.rpc.state;
import utils.system.system;

namespace core::rpc::endpoints::clipboard {

auto handle_read_text([[maybe_unused]] core::AppState& app_state,
                      [[maybe_unused]] const EmptyParams& params)
    -> RpcAwaitable<std::optional<std::string>> {
  auto result = utils::system::read_clipboard_text();
  if (!result) {
    co_return std::unexpected(
        RpcError{.code = static_cast<int>(ErrorCode::ServerError),
                 .message = "Failed to read clipboard text: " + result.error()});
  }

  co_return result.value();
}

auto register_all(core::AppState& app_state) -> void {
  register_method<EmptyParams, std::optional<std::string>>(
      app_state, app_state.rpc->registry, "clipboard.readText", handle_read_text,
      "Read plain text from the system clipboard");
}

}  // namespace core::rpc::endpoints::clipboard
