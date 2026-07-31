// P3a: can a named module wrap <windows.h> in its global module fragment and
// re-export the symbols into the GLOBAL namespace, so consumers keep writing
// plain `HWND` / `MessageBoxW` with no project-namespace wrapper?
//
// This is the strategy that would let the 60 vendor facades become modules
// while keeping the "vendor facades do not re-export through a project
// namespace" rule. `export using ::X;` names an entity already declared in
// the same scope; whether MSVC accepts it in a module purview is exactly the
// unknown. If this fails, p3c's strategy (each consumer includes the facade
// in its own GMF) is the fallback and costs nothing but re-parsing.
module;

#include <windows.h>

export module vendor.windows;

export using ::BOOL;
export using ::DWORD;
export using ::HWND;
export using ::HINSTANCE;
export using ::LPARAM;
export using ::LRESULT;
export using ::MSG;
export using ::POINT;
export using ::RECT;
export using ::UINT;
export using ::WPARAM;

export using ::GetLastError;
export using ::GetSystemMetrics;
export using ::MessageBoxW;
export using ::PeekMessageW;
