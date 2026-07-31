module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.gallery.tag;

import std;

export namespace core::rpc::endpoints::gallery::tag {

auto register_all(core::AppState& app_state) -> void;

}  // namespace core::rpc::endpoints::gallery::tag
