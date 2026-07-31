-- Asio (standalone) — the project's whole async runtime: core::async is an
-- Asio coroutine executor and every RPC handler returns asio::awaitable<>.
--
-- WHY NOT `chriskohlhoff.asio` FROM THE OFFICIAL INDEX. That package exposes
-- Asio as a C++23 module and compiles a module unit (asio.m.obj) with its own
-- default flags, i.e. /MD. This project must link /MT (mcpp's cached `std`
-- object is /MT, see the root mcpp.toml), and a consumer cannot override a
-- dependency's compile flags — the result is LNK2038 on every single object.
-- Since the project consumes Asio as headers anyway (src/vendor/asio.hpp is
-- `#include <asio.hpp>`, not `import asio`), a header-only package with the
-- CRT pinned is both simpler and sufficient.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "asio",
    description = "Asio (standalone, header-only): cross-platform async I/O and coroutines",
    licenses    = { "BSL-1.0" },
    repo        = "https://github.com/chriskohlhoff/asio",
    type        = "package",

    xpm = {
        windows = {
            ["1.38.1"] = {
                url    = "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-38-1.tar.gz",
                sha256 = "2827b229972be80cdb14e5497962fa393d1adf036b5869e2b9c99f644daadacc",
            },
        },
        linux = {
            ["1.38.1"] = {
                url    = "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-38-1.tar.gz",
                sha256 = "2827b229972be80cdb14e5497962fa393d1adf036b5869e2b9c99f644daadacc",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*/include" },
        -- Standalone Asio: no Boost. Header-only, so a trivial anchor TU gives
        -- mcpp a buildable lib target.
        defines      = { "ASIO_STANDALONE" },
        generated_files = {
            ["mcpp_generated/asio_anchor.c"] = "int mcpp_sm_asio_headers_anchor(void) { return 0; }\n",
        },
        sources      = { "mcpp_generated/asio_anchor.c" },
        targets      = { ["asio"] = { kind = "lib" } },
        deps         = { },
        -- See the note in every sibling descriptor: the CRT must match the
        -- consuming project's /MT.
        windows      = { cflags = { "/MT" }, cxxflags = { "/MT" }, ldflags = { "ws2_32.lib", "mswsock.lib" } },
    },
}
