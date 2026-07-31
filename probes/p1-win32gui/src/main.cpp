// P1: a real Win32 GUI executable — WinMain entry, /SUBSYSTEM:WINDOWS, and
// system libraries linked via ldflags. Creates a window, pumps a few messages,
// exits 0. Headless-safe: never blocks on input.
#include <windows.h>
#include <dwmapi.h>

namespace {

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  if (msg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = wnd_proc;
  wc.hInstance = inst;
  wc.lpszClassName = L"P1ProbeWindow";
  if (!RegisterClassExW(&wc)) return 1;

  HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"p1", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT, 320, 200,
                              nullptr, nullptr, inst, nullptr);
  if (!hwnd) return 2;

  // Touch a dwmapi symbol so the library is genuinely required at link time.
  BOOL enabled = FALSE;
  DwmIsCompositionEnabled(&enabled);

  MSG msg{};
  for (int i = 0; i < 16 && PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE); ++i) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  DestroyWindow(hwnd);
  return 0;
}
