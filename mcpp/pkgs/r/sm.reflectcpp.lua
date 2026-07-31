-- reflect-cpp — the serialization layer core::rpc is built on. Every RPC
-- request/response type is (de)serialized through rfl, and rfl is what
-- performs the snake_case <-> camelCase field-name conversion between C++ and
-- JSON, so this is load-bearing for the whole frontend protocol.
--
-- Only the core and JSON backends are compiled. Upstream ships one umbrella TU
-- per backend (reflectcpp.cpp #includes the five core rfl/*.cpp; the avro /
-- bson / capnproto / cbor / msgpack / toml / xml / yaml siblings each pull in
-- an external library the project does not use). yyjson is vendored by
-- reflect-cpp itself: rfl/json/save.hpp probes `__has_include(<yyjson.h>)` and
-- otherwise falls back to include/rfl/thirdparty/yyjson.h, so exposing only
-- include/ keeps the bundled copy in play and needs no external yyjson.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "reflectcpp",
    description = "reflect-cpp: C++ reflection-based serialization (core + JSON backend)",
    licenses    = { "MIT" },
    repo        = "https://github.com/getml/reflect-cpp",
    type        = "package",

    xpm = {
        windows = {
            ["0.25.0"] = {
                url    = "https://github.com/getml/reflect-cpp/archive/refs/tags/v0.25.0.tar.gz",
                sha256 = "de74d3793fd3dde9105ebe0f40bffb28df7009d59e0714389e4d29fcb46a1a3f",
            },
        },
        linux = {
            ["0.25.0"] = {
                url    = "https://github.com/getml/reflect-cpp/archive/refs/tags/v0.25.0.tar.gz",
                sha256 = "de74d3793fd3dde9105ebe0f40bffb28df7009d59e0714389e4d29fcb46a1a3f",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*/include" },
        sources      = {
            "*/src/reflectcpp.cpp",       -- umbrella: the five core rfl/*.cpp
            "*/src/reflectcpp_json.cpp",  -- umbrella: rfl/json/{Writer,to_schema}.cpp
            "*/src/yyjson.c",             -- vendored yyjson implementation
        },
        targets      = { ["reflectcpp"] = { kind = "lib" } },
        deps         = { },
    },
}
