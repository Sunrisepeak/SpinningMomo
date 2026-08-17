-- SQLiteCpp — the thin C++ RAII wrapper the project's core::database is
-- written against (`#include <SQLiteCpp/SQLiteCpp.h>`).
--
-- Upstream vendors sqlite3 as a git submodule, which a source tarball does not
-- carry; the dependency on sm.sqlite3 replaces it.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "sqlitecpp",
    description = "SQLiteCpp: smart and easy to use C++ SQLite3 wrapper",
    licenses    = { "MIT" },
    repo        = "https://github.com/SRombauts/SQLiteCpp",
    type        = "package",

    xpm = {
        windows = {
            ["3.3.3"] = {
                url    = "https://github.com/SRombauts/SQLiteCpp/archive/refs/tags/3.3.3.tar.gz",
                sha256 = "33bd4372d83bc43117928ee842be64d05e7807f511b5195f85d30015cad9cac6",
            },
        },
        linux = {
            ["3.3.3"] = {
                url    = "https://github.com/SRombauts/SQLiteCpp/archive/refs/tags/3.3.3.tar.gz",
                sha256 = "33bd4372d83bc43117928ee842be64d05e7807f511b5195f85d30015cad9cac6",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        include_dirs = { "*/include" },
        sources      = { "*/src/*.cpp" },
        targets      = { ["SQLiteCpp"] = { kind = "lib" } },
        -- Upstream vendors sqlite3 as a git submodule that the source tarball
        -- does not carry; the official index's amalgamation package replaces it.
        deps         = { ["compat.sqlite3"] = "3.45.3" },
    },
}
