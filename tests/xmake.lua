add_requires("vcpkg::doctest")

target("SpinningMomoTests")
    set_kind("binary")
    set_default(false)
    set_plat("windows")
    set_arch("x64")

    add_defines("NOMINMAX", "UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN",
                "_WIN32_WINNT=0x0A00")
    add_includedirs("../src")

    -- 被测的两个模块，接口与实现都要在这个 target 里。模块实现单元脱离自己的接口
    -- 编不了，所以 `.cppm` 不是可选的补充而是前提。
    --
    -- 只需要这两个：`scripts/check-module-graph.py` 算过它们的传递闭包，两个都是
    -- 叶子(除 `import std;` 外无项目依赖)。真要长出依赖，闭包会变大而这里会编译失败，
    -- 那时该扩这张清单而不是绕过它。
    add_files("../src/features/recording/time.cppm")
    add_files("../src/features/recording/time.cpp")
    add_files("../src/utils/path/path.cppm")
    add_files("../src/utils/path/path.cpp")
    add_files("test_main.cpp")
    add_files("features/recording/time_test.cpp")
    add_files("utils/path_test.cpp")

    add_packages("vcpkg::doctest")
    add_links("shell32", "ole32")
    add_tests("default")

target("SpinningMomoScenarioWindow")
    set_kind("binary")
    set_default(false)
    set_plat("windows")
    set_arch("x64")

    add_defines("NOMINMAX", "UNICODE", "_UNICODE", "WIN32_LEAN_AND_MEAN",
                "_WIN32_WINNT=0x0A00")
    add_includedirs("../src")
    add_files("scenarios/window/main.cpp")
    add_links("gdi32", "shell32", "user32")
