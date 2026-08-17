// `import asio;` and a TEXTUAL <windows.h> in the same translation unit.
//
// This is the shape most of SpinningMomo has: Win32 code that also does async
// I/O. It broke at [749/865] of the full build with
//
//   winuser.h:3835: error: conflicting types for 'BroadcastSystemMessageExA'
//   winuser.h:3835: note: previous declaration is here      <- the SAME line
//
// "the same line declares a conflicting type" means <windows.h> was parsed
// TWICE with different macro state and clang could not merge the results. The
// two parses are:
//
//   1. inside the asio module's global module fragment, when the PACKAGE was
//      compiled — with the package's defines, which do not include
//      `_WIN32_WINNT`. asio/detail/config.hpp then falls back to its own
//      default of 0x0601 (Windows 7);
//   2. here, textually, with the project's `_WIN32_WINNT=0x0A00`.
//
// asio defines WIN32_LEAN_AND_MEAN and NOMINMAX itself, so those two agree;
// the API level does not. The fix is to give the package the consumer's value —
// see the `windows` block in mcpp/pkgs/a/sm.asio.lua.
import std;
import asio;

#include <windows.h>

int main() {
    // Win32, from the textual include.
    const int cx = ::GetSystemMetrics(SM_CXSCREEN);
    if (cx <= 0) return 1;

    // …and the API level really is the one this TU asked for. Anything below
    // 0x0A00 would mean the define did not survive.
    static_assert(_WIN32_WINNT == 0x0A00);

    // Asio, from the module, in the same TU.
    asio::io_context io;
    bool ran = false;
    asio::post(io, [&] { ran = true; });
    io.run();
    if (!ran) return 2;

    // A type that exists in both worlds: asio's error_code is std::error_code,
    // and GetLastError feeds the Win32 category.
    const std::error_code ec(static_cast<int>(ERROR_FILE_NOT_FOUND), std::system_category());
    if (!ec) return 3;

    std::println("p8: import asio + textual <windows.h> coexist");
    return 0;
}
