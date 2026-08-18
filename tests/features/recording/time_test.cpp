// 见 tests/utils/path_test.cpp 顶部关于 mcpp test 形态的说明。
#include "vendor/std.hpp"

import sm.features.recording.time;

using features::recording::time::relative_timestamp_100ns;

namespace check {

int failures = 0;

void that(bool ok, const char* expression, int line) {
  if (ok) {
    return;
  }
  std::fprintf(stderr, "time_test.cpp:%d: FAIL  %s\n", line, expression);
  ++failures;
}

}  // namespace check

#define CHECK(expr) check::that((expr), #expr, __LINE__)

// 无效录制起点不能生成可供编码器使用的时间线
void invalid_recording_start_yields_zero() {
  CHECK(relative_timestamp_100ns(0, 100) == 0);
  CHECK(relative_timestamp_100ns(-1, 100) == 0);
}

// 起点之前的采样统一钳制为零，避免产生负时间戳
void timestamp_never_becomes_negative() {
  CHECK(relative_timestamp_100ns(100, 99) == 0);
  CHECK(relative_timestamp_100ns(100, 100) == 0);
}

// 有效采样使用统一的录制起点换算相对偏移
void timestamp_is_the_difference_from_recording_start() {
  CHECK(relative_timestamp_100ns(100, 135) == 35);
}

int main() {
  invalid_recording_start_yields_zero();
  timestamp_never_becomes_negative();
  timestamp_is_the_difference_from_recording_start();

  if (check::failures != 0) {
    std::fprintf(stderr, "%d check(s) failed\n", check::failures);
    return 1;
  }
  return 0;
}
