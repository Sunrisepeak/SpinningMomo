module;

#include "core/state/app_state.hpp"
#include "vendor/asio.hpp"
#include "core/rpc/types.hpp"

module core.rpc.endpoints.tasks.tasks;

import std;
import core.rpc.rpc;
import core.rpc.state;
import core.tasks.tasks;

namespace core::rpc::endpoints::tasks {

struct ClearFinishedTasksResult {
  std::int32_t cleared_count = 0;
};

auto handle_list_tasks(core::AppState& app_state, [[maybe_unused]] const EmptyParams& params)
    -> RpcAwaitable<std::vector<core::tasks::TaskSnapshot>> {
  co_return core::tasks::list_tasks(app_state);
}

auto handle_clear_finished_tasks(core::AppState& app_state,
                                 [[maybe_unused]] const EmptyParams& params)
    -> RpcAwaitable<ClearFinishedTasksResult> {
  co_return ClearFinishedTasksResult{
      .cleared_count = static_cast<std::int32_t>(core::tasks::clear_finished_tasks(app_state)),
  };
}

auto register_all(core::AppState& app_state) -> void {
  register_method<EmptyParams, std::vector<core::tasks::TaskSnapshot>>(
      app_state, app_state.rpc->registry, "task.list", handle_list_tasks,
      "List recent background tasks");

  register_method<EmptyParams, ClearFinishedTasksResult>(
      app_state, app_state.rpc->registry, "task.clearFinished", handle_clear_finished_tasks,
      "Clear finished background tasks");
}

}  // namespace core::rpc::endpoints::tasks
