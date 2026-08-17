add_rules("mode.debug", "mode.release")

-- 引入自定义任务
includes("tasks/release.lua")
includes("tasks/vs.lua")
includes("tests")

-- 设置C++23标准
set_languages("c++23")

-- 具名模块。src/ 下的 .cppm 与 third_party/asio-module/asio.cppm 都是模块单元，
-- 依赖图由 xmake 自己扫描 —— 与 mcpp 侧看到的是同一份源码形态。
set_policy("build.c++.modules", true)

-- 默认使用 LLVM 工具链，可通过 --toolchain 覆盖
set_config("toolchain", "clang-cl[llvm]")

-- 统一源文件编码。/bigobj 在 cl.exe 下是必需的：reflect-cpp 每个 RPC 端点实例化的
-- 模板足以撞破 COFF 的 65536 段上限。clang-cl 接受它但当空操作 —— LLVM 的 COFF
-- writer 段数超限时自动改用 big-object 格式，这也是 mcpp 侧(clang++ driver)根本
-- 没有对应拼法的原因。
add_cxflags("/utf-8", "/bigobj")

-- 设置运行时库
set_runtimes(is_mode("debug") and "MD" or "MT")

set_policy("package.requires_lock", true)

-- 锁定 vcpkg 注册表快照（2026-05-21）
add_requireconfs("vcpkg::*", {configs = {baseline = "1ea949145db9db7c9b254062f94acdaeed947767"}})

-- 添加vcpkg依赖包
add_requires("vcpkg::uwebsockets", "vcpkg::spdlog", "vcpkg::asio", "vcpkg::reflectcpp", 
             "vcpkg::webview2", "vcpkg::wil", "vcpkg::xxhash", "vcpkg::sqlitecpp", "vcpkg::libwebp", "vcpkg::zlib")

target("SpinningMomo")
    -- 设置为Windows可执行文件
    set_kind("binary")
    set_plat("windows")
    set_arch("x64")
    -- 具名模块与预编译头不能共存：PCH 是文本快照，模块单元的 purview 里不允许
    -- #include，两者对同一份 SDK 头会给出不同的实体归属。mcpp 侧根本没有 PCH，
    -- 这里也一并去掉，两个构建系统看到的是同一份源码形态。
    add_cxflags("clang_cl::-Wno-microsoft-include")

    -- Release 也保留调试符号，便于分析生产崩溃 dump
    if is_mode("release") then
        set_symbols("debug")
        add_ldflags("/DEBUG:FULL", {force = true})
        add_ldflags("/NODEFAULTLIB:libucrt.lib", {force = true})
        add_ldflags("/DEFAULTLIB:ucrt.lib", {force = true})
    end
    
    -- Windows特定宏定义
    add_defines("NOMINMAX", "UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN", "_WIN32_WINNT=0x0A00", "SPDLOG_COMPILED_LIB", "yyjson_api_inline=yyjson_inline")

    -- asio 的分离编译契约。这五个宏在 mcpp 侧由 chriskohlhoff.asio 的默认
    -- feature 提供，这里必须手写出来 —— 模块单元与消费者 TU 都要看到同一套，
    -- 否则消费者会把包已经编成 out-of-line 的实现再 inline 展开一遍。
    --
    -- _WIN32_WINNT 已在上面声明，且这里天然没有 mcpp 侧那个问题：asio 的模块单元
    -- 是**在工程内**编译的，与其他 TU 共享同一份宏状态。mcpp 侧它在包里编译，
    -- 所以要在描述符里单独钉一遍(见 mcpp/pkgs/a/sm.asio.lua)。
    add_defines("ASIO_STANDALONE", "ASIO_SEPARATE_COMPILATION",
                "ASIO_DISABLE_BOOST_CONTEXT_FIBER", "ASIO_HAS_THREADS", "ASIO_NO_IOSTREAM")

    -- uSockets 的后端/SSL 形态会改变 libusockets.h 里 us_loop_t 的布局，
    -- 每个看到 uWS 头文件的 TU 都必须与库的构建方式一致。
    add_defines("LIBUS_USE_LIBUV", "LIBUS_NO_SSL", "UWS_NO_ZLIB")
    
    -- 添加包含目录
    add_includedirs("src")
    add_includedirs("third_party/dkm/include")
    
    -- 添加源文件
    add_files("src/main.cpp")
    add_files("src/**.cpp")
    add_files("src/**.cppm")
    add_files("resources/*.rc")

    -- asio 的模块单元。项目只以 `import asio;` 消费 asio —— 那是阻止它的模板特化
    -- 在每个导入方重新实例化(C1116)的机制。mcpp 侧这个模块由索引包
    -- chriskohlhoff.asio 提供；vcpkg 只发头文件，所以 xmake 侧必须自己编。
    --
    -- asio.cppm 是那个包生成物的**逐字镜像**，asio_src.cpp 是
    -- ASIO_SEPARATE_COMPILATION 要求的那唯一一个实现 TU(mcpp 侧由包编译
    -- `*/src/asio.cpp`)。两份 wrapper 会漂移，所以
    -- scripts/check-asio-module-parity.py 负责 diff 它们。
    add_files("third_party/asio-module/asio.cppm")
    add_files("third_party/asio-module/asio_src.cpp")
    
    -- 链接vcpkg包
    add_packages("vcpkg::uwebsockets", "vcpkg::spdlog", "vcpkg::asio", "vcpkg::reflectcpp", 
                 "vcpkg::webview2", "vcpkg::wil", "vcpkg::xxhash", "vcpkg::sqlitecpp", "vcpkg::libwebp", "vcpkg::zlib")
    
    -- Windows系统库
    add_links("dwmapi", "dcomp", "windowsapp", "RuntimeObject", "d3d11", "dxgi", "d3dcompiler", 
              "d2d1", "dwrite", "shell32", "Shlwapi", "gdi32", "user32", "Ws2_32", "Secur32", 
              "Advapi32", "Bcrypt", "Dbghelp", "Userenv", "mf", "mfplat", "mfreadwrite", "mfuuid", "strmiids")

    -- vcpkg的传递依赖
    add_links("fmt", "yyjson", "sqlite3", "uSockets", "libuv")
