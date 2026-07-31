-- uSockets — the C transport layer under uWebSockets.
--
-- Backend: libuv (LIBUS_USE_LIBUV). The epoll/kqueue backend is POSIX-only and
-- GCD is macOS; uSockets 0.8.8 also ships an asio backend, which would fold
-- into the project's existing asio dependency, but libuv is what vcpkg's
-- uwebsockets port used, so it keeps core::http_server's runtime behaviour
-- identical across the migration.
--
-- SSL: off (LIBUS_NO_SSL). The project terminates no TLS in-process — the RPC
-- server binds localhost:51206 for the WebView2 frontend — so crypto/openssl.c
-- and crypto/sni_tree.cpp are excluded along with the OpenSSL dependency they
-- would drag in. quic.c and io_uring/ are likewise out of scope (lsquic /
-- Linux-only).
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "usockets",
    description = "uSockets: eventing and networking layer under uWebSockets (libuv backend, no SSL)",
    licenses    = { "Apache-2.0" },
    repo        = "https://github.com/uNetworking/uSockets",
    type        = "package",

    xpm = {
        windows = {
            ["0.8.8"] = {
                url    = "https://github.com/uNetworking/uSockets/archive/refs/tags/v0.8.8.tar.gz",
                sha256 = "d14d2efe1df767dbebfb8d6f5b52aa952faf66b30c822fbe464debaa0c5c0b17",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        -- src/ carries the public <libusockets.h>; internal headers are
        -- included flat from within it.
        include_dirs = { "*/src" },
        sources      = {
            "*/src/bsd.c",
            "*/src/context.c",
            "*/src/loop.c",
            "*/src/socket.c",
            "*/src/udp.c",
            "*/src/eventing/libuv.c",
        },
        -- Both flags are INTERFACE-level facts, not private ones: libusockets.h
        -- changes the shape of us_loop_t under LIBUS_USE_LIBUV and gates the
        -- SSL declarations on LIBUS_NO_SSL, so every consumer TU must see the
        -- same pair or the struct layouts disagree at link time.
        cflags       = { "-DLIBUS_USE_LIBUV", "-DLIBUS_NO_SSL" },
        cxxflags     = { "-DLIBUS_USE_LIBUV", "-DLIBUS_NO_SSL" },
        targets      = { ["uSockets"] = { kind = "lib" } },
        deps         = { ["sm.libuv"] = "1.52.1" },
        -- The consuming project links /MT (and mcpp's cached `std` object is
        -- /MT too), but a dependency package compiles with its own flags and
        -- would otherwise default to /MD — LNK2038 at the final link.
        windows      = { cflags = { "/MT" }, cxxflags = { "/MT" }, ldflags = { "ws2_32.lib" } },
    },
}
