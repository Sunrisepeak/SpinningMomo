module;

#include "core/state/app_state.hpp"

export module core.rpc.endpoints.gallery.gallery;

import std;

export namespace core::rpc::endpoints::gallery {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::gallery
