-- libuv — required only as uSockets' event-loop backend on Windows. uSockets
-- ships four backends (epoll/kqueue, GCD, libuv, asio); only libuv is viable
-- on Windows, and it is the same backend vcpkg's uwebsockets port used, so the
-- runtime behaviour of core::http_server does not change with the migration.
--
-- Sources follow libuv's own split: src/*.c is the portable core and src/win/*.c
-- is the platform layer (src/unix/*.c is its counterpart and is never built
-- here — this package is declared windows-only, like everything it exists for).
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "libuv",
    description = "libuv: cross-platform asynchronous I/O (uSockets' Windows event loop)",
    licenses    = { "MIT" },
    repo        = "https://github.com/libuv/libuv",
    type        = "package",

    xpm = {
        windows = {
            ["1.52.1"] = {
                url    = "https://github.com/libuv/libuv/archive/refs/tags/v1.52.1.tar.gz",
                sha256 = "478baf2599bfbc882c355288c9cb6f92e0e7dda435fa04031fa5b607cf3f414c",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        -- include/ is the public surface (<uv.h>); src/ is needed because the
        -- implementation includes "uv-common.h" and "win/internal.h" flat.
        include_dirs = { "*/include", "*/src" },
        sources      = {
            "*/src/fs-poll.c", "*/src/idna.c", "*/src/inet.c", "*/src/random.c",
            "*/src/strscpy.c", "*/src/strtok.c", "*/src/thread-common.c",
            "*/src/threadpool.c", "*/src/timer.c", "*/src/uv-common.c",
            "*/src/uv-data-getter-setters.c", "*/src/version.c",
            "*/src/win/*.c",
        },
        targets      = { ["uv"] = { kind = "lib" } },
        deps         = { },
        -- The consuming project links /MT (and mcpp's cached `std` object is
        -- /MT too), but a dependency package compiles with its own flags and
        -- would otherwise default to /MD — LNK2038 at the final link.
        windows = { cflags = { "/MT" }, cxxflags = { "/MT" },
            -- libuv's documented Windows link set.
            ldflags = {
                "psapi.lib", "user32.lib", "advapi32.lib", "iphlpapi.lib",
                "userenv.lib", "ws2_32.lib", "dbghelp.lib", "ole32.lib",
                "shell32.lib",
            },
        },
    },
}
