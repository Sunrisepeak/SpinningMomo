#pragma once

// uWebSockets' headers live flat in the upstream tree and are included as
// <App.h>. Distributions disagree about where to install them, and the two
// package managers this project builds under land on opposite sides:
//
//   mcpp   compat.uwebsockets exposes `*/src`, i.e. the upstream spelling
//   xmake  vcpkg installs under `include/uwebsockets/`
//
// This is the one place in the tree that has to know, which is what the vendor
// facade layer is for. It is a probe, not a preprocessor switch on the build
// system: whichever header is actually reachable wins.
#if __has_include(<App.h>)
#include <App.h>
#else
#include <uwebsockets/App.h>
#endif
