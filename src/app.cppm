module;

#include "vendor/windows.hpp"

export module sm.app;

import std;
import sm.core.events.events;
import sm.core.state.app_state;

// 主应用程序类
class Application {
 public:
  Application();
  ~Application();

  // 禁用拷贝和移动
  Application(const Application&) = delete;
  auto operator=(const Application&) -> Application& = delete;
  Application(Application&&) = delete;
  auto operator=(Application&&) -> Application& = delete;

  // 初始化
  [[nodiscard]] auto Initialize(HINSTANCE hInstance) -> bool;

  // 运行应用程序
  [[nodiscard]] auto Run() -> int;

 private:
  // 应用状态
  core::AppState m_app_state{};
};
