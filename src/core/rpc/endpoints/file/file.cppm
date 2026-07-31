module;

#include "core/state/app_state.hpp"

export module sm.core.rpc.endpoints.file.file;

import std;

export namespace core::rpc::endpoints::file {

auto register_all(core::AppState& state) -> void;

}  // namespace core::rpc::endpoints::file
