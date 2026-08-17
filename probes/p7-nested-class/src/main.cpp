// The consumer instantiates the template ACROSS the module boundary, which is
// what forces the compiler to find `Outer::Inner` in the imported BMI.
//
// PASS  -> the toolchain round-trips an out-of-line nested class definition
//          through a BMI, and `import asio;` is viable on it.
// FAIL  -> expect `error C2039: 'Inner': is not a member of 'demo::Outer'`
//          followed by `C2504: base class undefined` — the same pair
//          p6-asio-module hits at asio/io_context.hpp(1041).
import std;
import nestedwrap;

int main() {
    demo::Derived<int> d;
    if (d.value != 7) return 1;
    if (d.marker != 0) return 2;

    demo::Derived<double> e;
    e.marker = 0.5;
    if (e.value != 7) return 3;

    static_assert(std::is_base_of_v<demo::Outer::Inner, demo::Derived<int>>);

    std::println("p7: out-of-line nested class survived the BMI");
    return 0;
}
