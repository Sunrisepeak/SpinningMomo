// A module that wraps the header in its global module fragment and re-exports
// the names — the same shape `chriskohlhoff.asio`'s generated `asio.cppm` has.
module;

#include "nested.hpp"

export module nestedwrap;

export namespace demo {
using demo::Outer;
using demo::Derived;
}
