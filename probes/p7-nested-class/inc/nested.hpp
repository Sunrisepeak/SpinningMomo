#pragma once

// Reducing asio's `io_context::service` shape, to find out what exactly native
// cl.exe cannot round-trip through a BMI.
//
// ROUND 1 (three ingredients) PASSED under msvc, so it was NOT faithful:
//   * a nested class declared inside its enclosing class,
//   * defined afterwards at namespace scope,
//   * used as the base of a class template instantiated across the boundary.
//
// ROUND 2 adds the ingredient asio has and that reduction did not: the nested
// class SHADOWS a same-named nested class inherited from a base. In asio,
//
//   class execution_context          { public: class service; };
//   class execution_context::service { … };                       // out of line
//   class io_context : public execution_context { public: class service; };
//   class io_context::service : public execution_context::service { … };
//   template <typename T> class service_base : public io_context::service { … };
//
// so `service` names two different classes depending on which scope you ask,
// and the derived one is defined out of line in terms of the base one. The
// error cl.exe reports is a LOOKUP failure — `'service': is not a member of
// 'asio::io_context'` — which is what makes shadowing the suspect.
namespace demo {

class Base {
public:
    class Inner;                     // #1: the base's nested class
    int base_tag = 1;
};

class Base::Inner {                  // defined out of line
public:
    int base_value = 3;
};

class Outer : public Base {
public:
    class Inner;                     // #2: SHADOWS Base::Inner
    int outer_tag = 2;
};

class Outer::Inner : public Base::Inner {   // out of line, in terms of #1
public:
    int value = 7;
};

// The template asio's `detail::service_base` corresponds to. Instantiating it
// in the CONSUMER is what forces the compiler to resolve `Outer::Inner` out of
// the imported BMI — and to resolve it to #2 rather than the inherited #1.
template <typename T>
class Derived : public Outer::Inner {
public:
    T marker{};
};

}  // namespace demo
