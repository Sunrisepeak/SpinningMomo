// The consumer instantiates the template ACROSS the module boundary, which is
// what forces the compiler to find `Outer::Inner` in the imported BMI.
//
// PASS  -> the toolchain round-trips the shape, so it is not (yet) a faithful
//          reduction of what p6-asio-module hits.
// FAIL   -> expect `error C2039: 'Inner': is not a member of 'demo::Outer'`
//          followed by `C2504: base class undefined` — the same pair
//          p6-asio-module hits at asio/io_context.hpp(1041), at which point
//          this file is the upstream report.
//
// This probe is a RESEARCH ARTIFACT, not a gate: its msvc leg runs with
// continue-on-error precisely because either outcome is information.
import std;
import nestedwrap;

int main() {
    demo::Derived<int> d;
    if (d.value != 7) return 1;         // from Outer::Inner  (the shadowing one)
    if (d.base_value != 3) return 2;    // from Base::Inner   (its base)
    if (d.marker != 0) return 3;

    demo::Derived<double> e;
    e.marker = 0.5;
    if (e.value != 7) return 4;

    // The lookup that matters: `Outer::Inner` must resolve to the SHADOWING
    // nested class, not to the one inherited from Base.
    static_assert(std::is_base_of_v<demo::Outer::Inner, demo::Derived<int>>);
    static_assert(std::is_base_of_v<demo::Base::Inner, demo::Outer::Inner>);
    static_assert(!std::is_same_v<demo::Outer::Inner, demo::Base::Inner>);

    std::println("p7: shadowing out-of-line nested class survived the BMI");
    return 0;
}
