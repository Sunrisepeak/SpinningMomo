-- spdlog, compiled mode — the project's logging backend (utils::logger).
--
-- Same reason as sm.asio for not using `compat.spdlog` from the official
-- index: its objects compile /MD and this project links /MT, which a consumer
-- cannot override.
--
-- Compiled rather than header-only, matching what the project already asked
-- for: SPDLOG_COMPILED_LIB was in the xmake build's defines. That macro is an
-- INTERFACE fact — spdlog's own sources #error without it, AND every consumer
-- TU that includes a spdlog header must see it so the headers take the
-- extern-template path instead of re-emitting the implementation inline.
-- bundled_fmtlib_format.cpp compiles the vendored {fmt}, so no external fmt.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "spdlog",
    description = "spdlog: fast C++ logging library (compiled mode, bundled fmt)",
    licenses    = { "MIT" },
    repo        = "https://github.com/gabime/spdlog",
    type        = "package",

    xpm = {
        windows = {
            ["1.17.0"] = {
                url    = "https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz",
                sha256 = "d8862955c6d74e5846b3f580b1605d2428b11d97a410d86e2fb13e857cd3a744",
            },
        },
        linux = {
            ["1.17.0"] = {
                url    = "https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz",
                sha256 = "d8862955c6d74e5846b3f580b1605d2428b11d97a410d86e2fb13e857cd3a744",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*/include" },
        defines      = { "SPDLOG_COMPILED_LIB" },
        sources      = {
            "*/src/spdlog.cpp",
            "*/src/stdout_sinks.cpp",
            "*/src/color_sinks.cpp",
            "*/src/file_sinks.cpp",
            "*/src/async.cpp",
            "*/src/cfg.cpp",
            "*/src/bundled_fmtlib_format.cpp",
        },
        targets      = { ["spdlog"] = { kind = "lib" } },
        deps         = { },
        windows      = { cflags = { "/MT" }, cxxflags = { "/MT" } },
    },
}
