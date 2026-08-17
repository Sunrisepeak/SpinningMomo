// uWebSockets, as a module rather than a header.
//
// core::http_server uses it for the JSON-RPC endpoint and the SSE push channel
// the frontend talks to. The library is header-only C++ templated over
// uSockets' C types, so the three LIBUS_*/UWS_* defines that describe how
// uSockets was built have to reach this translation unit as well — they are in
// the build manifest, not here, because a facade cannot fix a contract the
// build has to state.
module;

// Distributions disagree about where uWebSockets' headers install: upstream
// keeps them flat in `src/` and includes them as <App.h>, which is what
// compat.uwebsockets exposes, while vcpkg installs under `uwebsockets/`. This
// probe is the one place in the tree that has to know.
#if __has_include(<App.h>)
#include <App.h>
#else
#include <uwebsockets/App.h>
#endif

export module sm.vendor.uwebsockets;

export namespace uWS {

using uWS::App;
using uWS::Loop;
using uWS::HttpResponse;
using uWS::HttpRequest;
using uWS::OpCode;

}  // namespace uWS

// uSockets' C surface reaches consumers through this facade too. uWS is a C++
// layer over it and hands its types straight back — `App::listen` yields a
// `us_listen_socket_t*` — so a consumer that only imported the uWS names would
// hold a type it cannot name:
//
//   core/http_server/state.cppm:13: error: missing '#include "libusockets.h"';
//   'us_listen_socket_t' must be declared before it is used
//
// Only what the project actually touches. The struct is opaque by design, so the
// declaration is all there is to export.
export {
using ::us_listen_socket_t;
using ::us_listen_socket_close;
}
