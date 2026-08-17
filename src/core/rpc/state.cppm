export module sm.core.rpc.state;

import std;
import sm.core.rpc.types;

export namespace core::rpc {

struct RpcState {
  std::unordered_map<std::string, MethodInfo> registry;
};

}  // namespace core::rpc
