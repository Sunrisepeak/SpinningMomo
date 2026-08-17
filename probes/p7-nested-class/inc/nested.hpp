#pragma once

// The exact shape Asio's io_context has, reduced to nothing else:
//
//   * a nested class DECLARED inside its enclosing class,
//   * DEFINED afterwards at namespace scope (`class Outer::Inner { … };`),
//   * used as the base of a class template.
//
// asio/io_context.hpp:211 declares `class service;`, :1005 defines
// `class io_context::service : public execution_context::service`, and :1039
// derives `template <typename T> class detail::service_base : public
// asio::io_context::service`. Instantiating any service — which `asio::post`,
// `steady_timer` and `make_strand` all do — walks that path.
namespace demo {

class Outer {
public:
    class Inner;                 // declared here …
    int tag = 1;
};

class Outer::Inner {             // … defined at namespace scope
public:
    int value = 7;
};

template <typename T>
class Derived : public Outer::Inner {
public:
    T marker{};
};

}  // namespace demo
