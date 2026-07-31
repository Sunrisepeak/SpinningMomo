module;

export module sm.core.dialog_service.state;

import std;

export namespace core::dialog_service {

struct DialogServiceState {
  std::jthread worker_thread;

  std::queue<std::move_only_function<void()>> task_queue;
  std::mutex queue_mutex;
  std::condition_variable condition;

  std::atomic<bool> is_running{false};
  std::atomic<bool> shutdown_requested{false};
};

}  // namespace core::dialog_service
