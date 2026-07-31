module;

#include "core/rpc/types.hpp"

export module sm.core.rpc.state;

import std;

export namespace core::rpc {

struct RpcState {
  std::unordered_map<std::string, MethodInfo> registry;
};

}  // namespace core::rpc
