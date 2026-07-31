module;

#include "core/state/app_state.hpp"

export module core.rpc.endpoints.gallery.asset;

import std;

export namespace core::rpc::endpoints::gallery::asset {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::gallery::asset
