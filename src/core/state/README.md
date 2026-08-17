# core::AppState

`AppState` 是本工程唯一的根对象：每个子系统一个 `std::unique_ptr<XxxState>`。
本文说明它的布局、依赖边界和维护约定。

## 结构

| 文件 | 职责 |
|------|------|
| `app_state.cppm` | 导出 `core::AppState`；`import` 29 个子系统 state 模块 |
| `app_state.cpp` | 集中构造和析构所有子 state |

各子系统 state 直接放在所属领域命名空间，例如：

```cpp
export namespace features::gallery {
struct GalleryState { … };
}
```

不要因为文件名是 `state.cppm` 就再造一个 `features::gallery::state` 命名空间。

## 前向声明没有了 —— 这是模块化强制的

头文件时代 `app_state.hpp` **前向声明**全部 29 个子 state，只保存
`std::unique_ptr<T>`，为的是不把整个世界拖进每个间接消费者的重编范围。

模块化之后这个手法不成立。`struct HttpServerState;` 写在这里，它就附着于
**本模块**；而定义附着于 `sm.core.http_server.state`。这是两个不同的实体，clang
直接说出来：

```
error: declaration 'HttpServerState' attached to named module
'sm.core.http_server.state' cannot be attached to other modules
```

于是 29 个前向声明变成 29 个 `import`。**没有丢失任何东西** —— 这条依赖一直都在，
只是以前没有写下来。重编范围的顾虑也随之消失：BMI 不是文本快照，改一个 state 的
私有字段不会让所有间接消费者重新解析它。

换来的代价是模块图上的一个真实的环被暴露出来（头文件的 include guard 一直藏着它）：

```
app_state -> ui.notification_window.state -> ui.notification_window.types
          -> core.notifications.types -> app_state
```

第三条回边是真的：一个 core 层的 types 模块依赖了应用根对象。见
`src/core/notifications/types.cppm` 里记录的处理方式。**一个 state 类型永远不需要
`AppState`** —— 这条由 `scripts/check-module-graph.py` 持续保证。

## API 约定

跨子系统入口优先接受：

```cpp
core::AppState&
const core::AppState&
```

实现单元再通过 `app_state.gallery->…` 访问具体状态。调用方不应穿透 `AppState`
之后把某个子 state 传给另一个领域。

应用完成初始化后的正常运行路径中，各 state 指针均由 `app_state.cpp` 创建，不需要在
每个业务函数里重复判空。

## 模板与非模板执行器

模板要写在**接口单元**（`.cppm`）里才能在导入方实例化。若模板体直接访问某个子 state
的字段，接口就必须 `import` 那个 state 模块，依赖范围会扩大。

模板应只完成类型映射，把需要访问具体 state、持锁或调度线程的工作委托给 `.cpp` 里的
非模板函数：

- `core::events` 的模板调用非模板事件执行器。
- `core::database` 的查询模板调用非模板数据库任务执行器。

只有真正依赖模板参数的逻辑才留在接口里。这也正是
`scripts/check-cpp-architecture.py` 那条「接口只放声明」检查放行模板的原因。

## 新增子 state

1. 在所属领域的 `state.cppm` 中 `export` 定义 `XxxState`。
2. 在 `app_state.cppm` 顶部按字典序加一行 `import sm.<领域>.state;`。
3. 在 `AppState` 中增加 `std::unique_ptr<XxxState>`。
4. 在 `app_state.cpp` 里 `import` 同一个模块并集中构造。
5. 对外能力继续接受 `core::AppState&`，不要暴露不必要的具体 state 依赖。
6. 跑一遍 `python3 scripts/check-module-graph.py` —— 新 state 若反过来依赖了
   `AppState`，这一步会红。

## 单元自包含

每个单元自己写全依赖，一个都不能靠别人顺带带进来：

- 模块单元写 `import std;`，普通 TU 包含 `vendor/std.hpp`（全工程只剩 `main.cpp`）；
- 用到的第三方走 `src/vendor/` 的封装模块或门面头；
- 用到的项目命名空间必须由自己 `import` 的某个模块导出 —— **`import` 不传递**，
  只有 `export import` 才重导出。

构建不依赖任何预编译头 —— `src/pch.hpp` 已随模块化删除（PCH 是文本快照，而模块单元的
purview 里不允许 `#include`）。
