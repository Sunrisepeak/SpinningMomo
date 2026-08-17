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
