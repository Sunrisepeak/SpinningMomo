-- uWebSockets — header-only C++ HTTP/WebSocket server. core::http_server uses
-- it for the JSON-RPC endpoint and the SSE push channel the Vue frontend talks
-- to when it runs in a browser during development.
--
-- Header-only, so a generated anchor TU gives mcpp a buildable lib target and
-- the real work is the include surface plus the uSockets dependency.
--
-- NOTE ON THE INCLUDE PATH: upstream's headers live in src/ and are included as
-- <App.h>; vcpkg installs them under uwebsockets/, which is why the project's
-- facade currently reads `#include <uwebsockets/App.h>`. Exposing */src here
-- means the facade changes to `#include <App.h>` — a one-line edit in
-- src/vendor/uwebsockets.hpp, which is the whole point of having the facade.
package = {
    spec        = "1",
    namespace   = "sm",
    name        = "uwebsockets",
    description = "uWebSockets: header-only HTTP/WebSocket server (no SSL)",
    licenses    = { "Apache-2.0" },
    repo        = "https://github.com/uNetworking/uWebSockets",
    type        = "package",

    xpm = {
        windows = {
            ["20.79.0"] = {
                url    = "https://github.com/uNetworking/uWebSockets/archive/refs/tags/v20.79.0.tar.gz",
                sha256 = "d255491a19c26b3f1593c686d4c07d7d2cebe1ba68d42caad87c068cfed0bf84",
            },
        },
    },

    mcpp = {
        schema       = "0.1",
        language     = "c++23",
        import_std   = false,
        c_standard   = "c11",
        include_dirs = { "*/src" },
        generated_files = {
            ["mcpp_generated/uws_anchor.c"] = "int mcpp_sm_uwebsockets_headers_anchor(void) { return 0; }\n",
        },
        sources      = { "mcpp_generated/uws_anchor.c" },
        -- Must match sm.usockets: uWS's templates instantiate over us_loop_t
        -- and friends, so the same backend/SSL shape has to be visible here.
        cxxflags     = { "-DLIBUS_USE_LIBUV", "-DLIBUS_NO_SSL", "-DUWS_NO_ZLIB" },
        targets      = { ["uWebSockets"] = { kind = "lib" } },
        deps         = { ["sm.usockets"] = "0.8.8" },
    },
}
