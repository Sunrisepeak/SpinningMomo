// A project module that reaches Win32 through its global module fragment —
// i.e. what `src/vendor/windows.hpp` would become.
//
// The consumer then never includes <windows.h> textually: it imports this and
// imports asio, so the Windows SDK reaches the TU only through BMIs. That is
// the variable p9 tests against p8.
module;

#include <windows.h>

export module winmod;

export {

// Types and functions carry across as ordinary using-declarations. The two
// below are exactly the ones that broke in p8: their parameter is an UNNAMED
// struct with a typedef name for linkage.
using ::FLASHWINFO;
using ::PFLASHWINFO;
using ::FlashWindowEx;

using ::HWND;
using ::BOOL;
using ::DWORD;
using ::HRESULT;
using ::GetSystemMetrics;
using ::GetLastError;

// MACROS DO NOT CROSS A MODULE BOUNDARY. This is the cost of the whole
// approach, and the reason `src/vendor/windows.hpp` is still a header in the
// real tree: `FAILED` (295 uses), `SUCCEEDED` (49), `IID_PPV_ARGS` (42),
// `WM_*` (~100) and friends would each need a constexpr replacement plus a
// rewrite of every call site. Two of them are re-declared here as constants so
// this probe can use them at all — which is precisely the work that would have
// to be done ~600 times.
}  // export

// A module name is not a namespace, so the constants get an explicit one rather
// than landing at global scope beside the SDK's own names.
export namespace winmod {
inline constexpr int kSmCxScreen = SM_CXSCREEN;
inline constexpr ::DWORD kFlashStop = FLASHW_STOP;
}  // namespace winmod
