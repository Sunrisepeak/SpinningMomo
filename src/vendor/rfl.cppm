// reflect-cpp, as a module rather than a header.
//
// This is the serialization layer core::rpc is built on: every RPC request and
// response type goes through it, and it is what performs the snake_case <->
// camelCase field-name conversion between C++ and the frontend protocol.
//
// Everything exported here is a TEMPLATE or a template's driver. What crosses
// the module boundary is the template; the instantiation still happens in the
// consumer, over the consumer's own struct — which is the only way reflection
// over a caller's type can work at all.
module;

#include <rfl.hpp>
#include <rfl/json.hpp>

export module sm.vendor.rfl;

export namespace rfl {

// The dynamic JSON value type. `Generic::Object` comes with it as a nested
// type, so it needs no separate export.
using rfl::Generic;

// Field-name processors. The project's protocol is camelCase on the wire and
// snake_case in C++, so SnakeCaseToCamelCase is on every read and write.
using rfl::SnakeCaseToCamelCase;
using rfl::DefaultIfMissing;

// The generic <-> typed bridge core::rpc uses when the concrete type is only
// known per method.
using rfl::to_generic;
using rfl::from_generic;
using rfl::to_view;

}  // namespace rfl

export namespace rfl::json {
using rfl::json::read;
using rfl::json::write;
using rfl::json::pretty;
using rfl::json::to_schema;
}  // namespace rfl::json
