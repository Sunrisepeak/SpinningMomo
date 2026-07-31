// P2: the binary itself is incidental; the assertion is that gen/app.res
// reached link.exe and its version resource survives into the PE.
#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) { return 0; }
