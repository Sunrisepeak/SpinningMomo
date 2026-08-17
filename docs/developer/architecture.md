# 架构与构建

> [!NOTE]
> 本项目更推荐先通过 Issue 讨论需求和方向，再提交 Pull Request。
>
> 对于新功能、行为调整或较大的重构，请先提交 Issue，说明要解决的问题、使用场景和预期效果。这样可以在开始编码前确认它是否符合项目定位。
>
> 已确认范围的 Issue、明确的 Bug 修复、文档改进，以及经过讨论的技术难题，都非常欢迎通过 PR 贡献。未经讨论的功能性 PR 可能会因为方向不一致而无法合并。

## 架构与代码规范说明

本项目核心采用 C++23 原生后端与 Vue 3 Web 前端的混合双端架构。C++ 后端是
**C++23 具名模块**：每个单元一个 `.cppm` 接口 + 同名 `.cpp` 实现，没有预编译头，
`src/vendor/` 之外没有任何项目头文件。

项目代码通过 `src/vendor/` 下的精确门面引入外部头。门面有两种形态，分界线是**宏**：
真正的第三方库（xxhash / webp / sqlite / spdlog / rfl / uWebSockets / dkm）是
`.cppm` 封装模块，头文件只在那里解析一次；Windows SDK、WIL、WebView2 仍是头文件，
由各模块在自己的全局模块片段里包含 —— 因为宏不进 BMI，而这棵树用了约 600 处
（`FAILED`、`SUCCEEDED`、`IID_PPV_ARGS`、`WM_*` 等）。

依赖来自**官方 mcpp 包索引**（`mcpplibs/mcpp-index`），项目不再自带包索引。

关于详细的设计哲学、C++ 组件划分、模块命名规则以及**十一条机器强制的架构不变量**，
见仓库根目录的 **[`AGENTS.md`](https://github.com/ChanIok/SpinningMomo/blob/main/AGENTS.md)**。

## 环境要求

C++ 后端有**两套构建，都是 record**：mcpp（主，依赖来自官方包索引）与 xmake + vcpkg。
两者都通过 **LLVM 走 MSVC ABI**，而不是原生 `cl.exe` —— 这是被 `import asio;` 逼出来的，
不是偏好：cl.exe 无法把 asio 的 `io_context::service`（类内声明、类外定义的嵌套类）
经 BMI 往返，而 asio 的模块形态正是阻止其模板特化在每个导入方重复实例化的机制。
`probes/p6-asio-module` 在两个工具链下各跑一遍，记录着这个差异。

| 工具 | 要求 | 说明 |
|------|------|------|
| **Visual Studio 2026 / Build Tools** | 安装「使用 C++ 的桌面开发」及 C++ Clang 工具 | Visual Studio IDE 可选 |
| **Windows SDK** | 10.0.22621.0+（Windows 11 SDK） | |
| **Git** | 最新版 | 克隆 vcpkg 与获取第三方依赖 |
| **mcpp** | 2026.8.17.1+ | 主构建系统（`xlings install mcpp`；`.xlings.json` 里钉了版本） |
| **xmake** | 3.1.0+ | 第二套构建系统 |
| **Node.js** | v20+ | Web 前端构建及 npm 脚本 |

### 安装 xmake

```powershell
# PowerShell（推荐）
irm https://xmake.io/psget.text | iex

# 或前往官网下载安装包
# https://xmake.io/#/getting_started?id=installation
```

### 准备 vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git D:\dev\vcpkg  # 路径自定
cd D:\dev\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe integrate install
```

---

## 依赖准备

### 1. 获取第三方依赖

```bash
npm run fetch:third-party
```

### 2. 安装 npm 依赖

```bash
# 根目录（构建脚本依赖）
npm install

# Web 前端依赖
npm ci --prefix web
```

### 3. 初始化 xmake 依赖并应用补丁

```bash
node scripts/patch-xmake-7554.js
node scripts/patch-xmake-clang-cl-cxx23.js

# Clang-cl + LLD（默认）
xmake f --toolchain="clang-cl[llvm]" -y

# 或使用 MSVC
# xmake f --toolchain=msvc -y

xmake f -m release -y && xmake f -m debug -y
node scripts/patch-vcpkg.js
```

---

## 使用 Visual Studio IDE 开发（可选）

如需使用 Visual Studio 浏览、编辑和调试 C++ 代码，可生成由 Xmake 管理的解决方案：

```powershell
xmake vs
```

生成后打开：

```text
vsxmake2026\SpinningMomo.sln
```

---

## 构建

> [!TIP]
> 如果在本地搭建或构建过程中遇到工具链、依赖或环境问题，建议参考 GitHub CI 的两个工作流 ——
> [`mcpp-windows.yml`](https://github.com/ChanIok/SpinningMomo/blob/main/.github/workflows/mcpp-windows.yml)
> 与 [`xmake-windows.yml`](https://github.com/ChanIok/SpinningMomo/blob/main/.github/workflows/xmake-windows.yml)
> —— 它们记录着当前自动化跑通的标准环境配置与构建顺序。

### 完整构建（推荐）

```bash
# 一键完成：C++ Release + Web 前端 + 打包 dist/
npm run build
```

产物位于 `dist/` 目录。

### 分步构建

```bash
# C++ 后端 —— mcpp（主）
mcpp build                 # debug
mcpp build --release

# C++ 后端 —— xmake + vcpkg（第二套）
xmake f -m debug --toolchain=clang-cl -y && xmake build
xmake release              # 构建 release 后自动恢复 debug 配置

# Web 前端
npm run build --prefix web

# 打包 dist/（汇总 exe + web 资源）
npm run build:prepare
```

### 推送前的秒级自检

这四项在 Linux/macOS/Windows 上都能跑，加起来不到一秒，而它们每挡下一次错误就省掉一轮
约 40 分钟的 Windows CI。`mcpp-windows.yml` 的 `preflight (linux)` job 跑的就是前三项，
`probe` 与 `full-build` 都 `needs` 它。

```bash
python3 scripts/check-module-graph.py        # 模块图是 DAG；且没有漏写的 import
python3 scripts/check-cpp-architecture.py    # 十一条架构不变量
python3 scripts/check-asio-module-parity.py  # xmake 侧的 asio 镜像与索引描述符一致
python3 scripts/check-build-parity.py        # 两套构建的宏集合与源码集合一致
bash    scripts/mcpp-preflight.sh            # 探针中 Linux 能判定的部分
```

### 构建输出路径

| 构建类型 | 路径 |
|----------|------|
| Debug | `build\windows\x64\debug\` |
| Release | `build\windows\x64\release\` |
| 打包产物 | `dist\` |

### 后端自动化测试

后端回归测试使用 doctest，并由独立的 `SpinningMomoTests` 目标承载：

```bash
xmake test -v
```

测试只保护确定性的稳定行为和已记录不变量，不以覆盖率为目标。涉及窗口、显卡、
音频设备和其他 Windows 桌面环境的行为仍需运行应用进行手工验证。

---

## 打包发布产物

### 便携版（ZIP）

```bash
npm run build:portable
```

### MSI 安装包

需要额外安装 WiX Toolset v6：

```bash
dotnet tool install --global wix --version 6.0.2
wix extension add WixToolset.UI.wixext/6.0.2 --global
wix extension add WixToolset.BootstrapperApplications.wixext/6.0.2 --global
```

然后运行：

```bash
npm run build:installer
```

---

## Web 前端开发

启动开发服务器（需 C++ 后端同时运行）：

```bash
npm run dev:web
```

Vite 开发服务器会将 `/rpc` 和 `/static` 代理到 C++ 后端（`localhost:51206`）。

---

## 代码生成脚本

修改以下源文件后需重新运行对应脚本：

| 修改内容 | 需运行的脚本 |
|----------|-------------|
| `src/migrations/*.sql` | `node scripts/generate-migrations.js` |
| `src/locales/*.json` | `node scripts/generate-embedded-locales.js` |
