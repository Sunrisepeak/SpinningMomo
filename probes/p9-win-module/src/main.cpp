// Win32 AND asio, both reached through modules — no textual <windows.h> here.
//
// p8 has the same two libraries in one TU with <windows.h> included textually.
// If p8 fails and this passes, the rule is "the Windows SDK must enter a TU
// exactly once, through a BMI", and the migration's next step is to turn
// src/vendor/windows.hpp into a module — at the cost of ~600 macro call sites
// (see the note at the bottom of winmod.cppm).
//
// If BOTH fail, the SDK cannot coexist with an asio BMI in one TU under clang
// at all, and `import asio;` has to be confined to translation units that do
// not touch Win32.
import std;
import asio;
import winmod;

int main() {
    const int cx = ::GetSystemMetrics(winmod::kSmCxScreen);
    if (cx <= 0) return 1;

    // The p8 casualty, reached entirely through the module.
    ::FLASHWINFO fwi{};
    fwi.cbSize = sizeof(::FLASHWINFO);
    fwi.hwnd = nullptr;
    fwi.dwFlags = winmod::kFlashStop;
    (void)::FlashWindowEx(&fwi);

    asio::io_context io;
    bool ran = false;
    asio::post(io, [&] { ran = true; });
    io.run();
    if (!ran) return 2;

    std::println("p9: Win32 and asio coexist when both arrive as modules");
    return 0;
}
