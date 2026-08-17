// `import asio;` and a TEXTUAL <windows.h> in the same translation unit.
//
// This is the shape most of SpinningMomo has: Win32 code that also does async
// I/O. It broke the full build at [749/865] with
//
//   winuser.h:3835: error: conflicting types for 'BroadcastSystemMessageExA'
//   winuser.h:3835: note: previous declaration is here      <- the SAME line
//
// <windows.h> is parsed twice in this TU: once inside asio's global module
// fragment (baked into the BMI when the PACKAGE was compiled) and once here,
// textually. Something about the two parses does not merge.
//
// ── ROUND 1: macro state.  REFUTED. ─────────────────────────────────────────
// The package build had no `_WIN32_WINNT`, so asio/detail/config.hpp fell back
// to its own default of 0x0601 while this TU asks for 0x0A00. Pinning the
// package to 0x0A00 put the define on the asio.cppm compile line — and the error
// did not move. So the API level is not it.
//
// ── ROUND 2: order.  What this file now tests. ──────────────────────────────
// The three failing functions have one thing in common — their parameter type:
//
//   BroadcastSystemMessageExA/W   PBSMINFO      typedef struct {…} BSMINFO, *PBSMINFO;
//   FlashWindowEx                 PFLASHWINFO   typedef struct {…} FLASHWINFO, *PFLASHWINFO;
//
// Both are UNNAMED structs carrying a typedef name for linkage. Parsed twice in
// one TU, clang can end up with two distinct types, and then every function
// taking one is a redeclaration with a different signature.
//
// Whether that resolves may depend on WHICH parse clang sees first: merging a
// BMI into existing textual declarations is a different path than the reverse.
// So <windows.h> now comes BEFORE the imports, and nothing else changed.
//
// A pass here means the project-wide rule is "all #include before all import"
// in a plain translation unit, which the conversion script can enforce.
// (It passed. The rule is now enforced by scripts/check-cpp-architecture.py.)
//
// ── ROUND 3: is the pin needed AT ALL? ──────────────────────────────────────
// Rounds 1 and 2 both carried the pin, so ordering was only ever shown to be
// sufficient WITH it. This round drops it: the manifest now depends on the
// published `chriskohlhoff.asio` rather than the project's override. Green means
// the include-before-import rule does all the work and the override can go,
// which is what P7 needs before the project can delete its `[indices]` table.
//
#include <windows.h>

import std;
import asio;

int main() {
    // Win32, from the textual include.
    const int cx = ::GetSystemMetrics(SM_CXSCREEN);
    if (cx <= 0) return 1;

    // The API level really is the one this TU asked for.
    static_assert(_WIN32_WINNT == 0x0A00);

    // One of the actual casualties: a function whose parameter is an unnamed
    // struct's typedef. Naming it is the whole point — if the two parses did
    // not merge, this line does not compile.
    FLASHWINFO fwi{};
    fwi.cbSize = sizeof(FLASHWINFO);
    fwi.hwnd = nullptr;
    fwi.dwFlags = FLASHW_STOP;
    (void)::FlashWindowEx(&fwi);   // returns FALSE for a null hwnd; that is fine

    // Asio, from the module, in the same TU.
    asio::io_context io;
    bool ran = false;
    asio::post(io, [&] { ran = true; });
    io.run();
    if (!ran) return 2;

    // A type that exists in both worlds.
    const std::error_code ec(static_cast<int>(ERROR_FILE_NOT_FOUND), std::system_category());
    if (!ec) return 3;

    std::println("p8: import asio + textual <windows.h> coexist (include-first)");
    return 0;
}
