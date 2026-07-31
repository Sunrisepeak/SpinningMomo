module;

#include "core/state/app_state.hpp"

export module core.rpc.endpoints.clipboard.clipboard;

import std;

export namespace core::rpc::endpoints::clipboard {

auto register_all(core::AppState& state) -> void;

}  // namespace core::rpc::endpoints::clipboard
