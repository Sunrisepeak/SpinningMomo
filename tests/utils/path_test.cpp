// mcpp test 编译 tests/**/*.cpp，一个文件一个二进制，用退出码判定成败。
// 框架无关 —— 所以这里就是一个 main 加一个计数器，没有第三方依赖。
#include "vendor/std.hpp"

import sm.utils.path.path;

using utils::path::ClassifyPathStorageKind;
using utils::path::IsPathWithinBase;
using utils::path::NormalizeForComparison;
using utils::path::PathStorageKind;
using utils::path::TryParseUncServer;

namespace check {

int failures = 0;

void that(bool ok, const char* expression, int line) {
  if (ok) {
    return;
  }
  std::fprintf(stderr, "path_test.cpp:%d: FAIL  %s\n", line, expression);
  ++failures;
}

}  // namespace check

#define CHECK(expr) check::that((expr), #expr, __LINE__)

// 标准 UNC 与扩展 UNC 路径都应解析出同一个服务器段
void unc_parser_accepts_supported_forms() {
  const auto standard_server = TryParseUncServer(LR"(\\server\share\photo.jpg)");
  const auto forward_slash_server = TryParseUncServer(L"//server/share/photo.jpg");
  const auto extended_server = TryParseUncServer(LR"(\\?\UNC\server\share\photo.jpg)");
  const auto device_server = TryParseUncServer(LR"(\\.\UNC\server\share\photo.jpg)");

  CHECK(standard_server.has_value());
  CHECK(forward_slash_server.has_value());
  CHECK(extended_server.has_value());
  CHECK(device_server.has_value());
  CHECK(standard_server.value_or(L"") == L"server");
  CHECK(forward_slash_server.value_or(L"") == L"server");
  CHECK(extended_server.value_or(L"") == L"server");
  CHECK(device_server.value_or(L"") == L"server");
}

// 本地路径和缺少共享名的网络地址不能被误判为可用 UNC 文件路径
void unc_parser_rejects_local_and_incomplete_paths() {
  CHECK(!TryParseUncServer(LR"(C:\Photos\photo.jpg)").has_value());
  CHECK(!TryParseUncServer(LR"(\\?\C:\Photos\photo.jpg)").has_value());
  CHECK(!TryParseUncServer(LR"(\\server)").has_value());
  CHECK(!TryParseUncServer(LR"(relative\photo.jpg)").has_value());
}

// 存储分类只把具备服务器与共享段的 UNC 路径视为远程位置
void storage_kind_distinguishes_unc_from_local() {
  CHECK(ClassifyPathStorageKind(LR"(\\server\share\photo.jpg)") == PathStorageKind::RemoteUnc);
  CHECK(ClassifyPathStorageKind(LR"(\\?\UNC\server\share\photo.jpg)") ==
        PathStorageKind::RemoteUnc);
  CHECK(ClassifyPathStorageKind(LR"(C:\Photos\photo.jpg)") == PathStorageKind::Local);
  CHECK(ClassifyPathStorageKind(LR"(\\?\C:\Photos\photo.jpg)") == PathStorageKind::Local);
  CHECK(ClassifyPathStorageKind(LR"(relative\photo.jpg)") == PathStorageKind::Local);
}

// 比较键统一 Windows 大小写、分隔符和 lexical 冗余段
void comparison_normalization_is_stable() {
  CHECK(NormalizeForComparison(LR"(C:\Photos\.\Album\..\A.JPG)") == LR"(c:/photos/a.jpg)");
}

// 相同路径与任意深度的子路径都属于扫描根
void containment_accepts_equal_and_descendant_paths() {
  CHECK(IsPathWithinBase(LR"(C:\Photos)", LR"(C:\Photos)"));
  CHECK(IsPathWithinBase(LR"(c:\PHOTOS)", LR"(C:\Photos)"));
  CHECK(IsPathWithinBase(LR"(C:\Photos\a.jpg)", LR"(C:\Photos)"));
  CHECK(IsPathWithinBase(LR"(C:\Photos\2026\July\a.jpg)", LR"(C:\Photos)"));
}

// 根路径和显式尾分隔符仍应保持正常的父子关系
void containment_handles_roots_and_trailing_separators() {
  CHECK(IsPathWithinBase(LR"(C:\Photos\a.jpg)", LR"(C:\)"));
  CHECK(IsPathWithinBase(LR"(C:\Photos\a.jpg)", LR"(C:\Photos\)"));
}

// 路径比较必须按完整路径段判断，不能只依赖字符串前缀
void containment_rejects_parents_siblings_and_similar_prefixes() {
  CHECK(!IsPathWithinBase(LR"(C:\PhotoArchive\a.jpg)", LR"(C:\Photo)"));
  CHECK(!IsPathWithinBase(LR"(C:\Photos)", LR"(C:\Photos\2026)"));
  CHECK(!IsPathWithinBase(LR"(D:\Photos\a.jpg)", LR"(C:\Photos)"));
}

int main() {
  unc_parser_accepts_supported_forms();
  unc_parser_rejects_local_and_incomplete_paths();
  storage_kind_distinguishes_unc_from_local();
  comparison_normalization_is_stable();
  containment_accepts_equal_and_descendant_paths();
  containment_handles_roots_and_trailing_separators();
  containment_rejects_parents_siblings_and_similar_prefixes();

  if (check::failures != 0) {
    std::fprintf(stderr, "%d check(s) failed\n", check::failures);
    return 1;
  }
  return 0;
}
