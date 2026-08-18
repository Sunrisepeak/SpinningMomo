export module sm.core.tasks.tasks;

import std;
import sm.core.state.app_state;
import sm.core.tasks.state;

export namespace core::tasks {

auto create_task(core::AppState& state, const std::string& type,
                 const std::optional<std::string>& context = std::nullopt) -> std::string;

auto has_active_task_of_type(core::AppState& state, const std::string& type) -> bool;

auto find_active_task_of_type(core::AppState& state, const std::string& type)
    -> std::optional<TaskSnapshot>;

auto mark_task_running(core::AppState& state, const std::string& task_id) -> bool;

auto update_task_progress(core::AppState& state, const std::string& task_id,
                          const TaskProgress& progress) -> bool;

auto complete_task_success(core::AppState& state, const std::string& task_id) -> bool;

auto complete_task_failed(core::AppState& state, const std::string& task_id,
                          const std::string& error_message) -> bool;

auto list_tasks(core::AppState& state) -> std::vector<TaskSnapshot>;

auto clear_finished_tasks(core::AppState& state) -> std::size_t;

}  // namespace core::tasks
