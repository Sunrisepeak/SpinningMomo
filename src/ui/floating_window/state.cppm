export module sm.ui.floating_window.state;

import std;
import sm.ui.floating_window.types;

export namespace ui::floating_window {

// 主窗口聚合状态
struct FloatingWindowState {
  floating_window::WindowInfo window;
  floating_window::InteractionState ui;
  floating_window::DataState data;
  floating_window::LayoutConfig layout;
  floating_window::RenderResources render_resources;  // 浮窗私有的窗口级渲染上下文
};

}  // namespace ui::floating_window
