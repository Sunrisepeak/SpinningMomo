// Asio's separate-compilation translation unit, for the xmake build.
//
// The project compiles with ASIO_SEPARATE_COMPILATION, which turns Asio's
// non-template implementation from inline definitions into ordinary ones that
// exactly one translation unit has to provide. Upstream ships that TU as
// `asio/src/asio.cpp`, whose entire content is this include; vcpkg installs the
// headers only, so the file has to exist here.
//
// Under mcpp the index package compiles `*/src/asio.cpp` itself, which is why
// there is no counterpart to this file on that side.
//
// This TU must NOT be a module unit: `asio/impl/src.hpp` defines the out-of-line
// implementations, and they belong to the global module — the same place the
// `asio` module's own global module fragment put their declarations.
#include <asio/impl/src.hpp>
