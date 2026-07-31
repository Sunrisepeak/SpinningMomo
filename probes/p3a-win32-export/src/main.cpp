import std;
import vendor.windows;

int main() {
  // Consumers see the Win32 names unqualified, exactly as with the header.
  const int cx = GetSystemMetrics(0 /*SM_CXSCREEN*/);
  RECT r{0, 0, 10, 20};
  const DWORD err = GetLastError();
  std::println("p3a: cx={} rect.right={} lasterr={}", cx, r.right, err);
  return 0;
}
