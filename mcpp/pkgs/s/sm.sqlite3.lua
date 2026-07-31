-- SQLite, from the official amalgamation. The project reaches it only through
-- SQLiteCpp, but the C library has to exist as its own package so the wrapper
-- can depend on it.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "sqlite3",
    description = "SQLite: self-contained SQL database engine (official amalgamation)",
    licenses    = { "blessing" },
    repo        = "https://www.sqlite.org",
    type        = "package",

    xpm = {
        windows = {
            ["3.50.4"] = {
                url    = "https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip",
                sha256 = "1d3049dd0f830a025a53105fc79fd2ab9431aea99e137809d064d8ee8356b032",
            },
        },
        linux = {
            ["3.50.4"] = {
                url    = "https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip",
                sha256 = "1d3049dd0f830a025a53105fc79fd2ab9431aea99e137809d064d8ee8356b032",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*" },
        sources      = { "*/sqlite3.c" },   -- shell.c is the CLI, not the library
        targets      = { ["sqlite3"] = { kind = "lib" } },
        deps         = { },
        -- Match what the project already asks of SQLite: the FTS/JSON features
        -- the gallery search path uses, and thread-safe serialized mode for the
        -- thread-local connection pool in core::database.
        cflags = {
            "-DSQLITE_ENABLE_FTS5",
            "-DSQLITE_ENABLE_JSON1",
            "-DSQLITE_ENABLE_COLUMN_METADATA",
            "-DSQLITE_THREADSAFE=1",
        },
        windows = { ldflags = { } },
    },
}
