#!/usr/bin/env bash
# Expose the Windows SDK's C++/WinRT projection headers under a space-free,
# version-independent path inside the project.
#
# Two mcpp gaps make this necessary:
#  1. The synthesised INCLUDE covers {ucrt,um,shared,winrt} only
#     (src/toolchain/msvc.cppm:457-460). The `winrt` dir there is the ABI
#     header set (lowercase windows.foundation.h); the C++ projection
#     (winrt/Windows.Foundation.h) lives in Include/<ver>/cppwinrt/ and is
#     never added.
#  2. An `include_dirs` entry containing spaces is emitted unquoted, so
#     "C:/Program Files (x86)/..." reaches cl.exe as three arguments and the
#     tail lands in the source-file position ("Cannot open source file:
#     'Files'"). So the path this project hands mcpp must not contain spaces.
# A directory junction satisfies both without copying ~100 MB of headers.
set -euo pipefail
cd "$(dirname "$0")"

link="gen/cppwinrt"
mkdir -p gen
[ -e "$link" ] && exit 0

src=""
for root in "/c/Program Files (x86)/Windows Kits/10/Include" "/c/Program Files/Windows Kits/10/Include"; do
  [ -d "$root" ] || continue
  cand=$(find "$root" -maxdepth 2 -type d -name cppwinrt 2>/dev/null | sort -V | tail -1)
  [ -n "$cand" ] && src="$cand" && break
done
[ -n "$src" ] || { echo "prebuild: no cppwinrt dir under any Windows Kit" >&2; exit 1; }
echo "prebuild: cppwinrt at $src"

MSYS2_ARG_CONV_EXCL='*' cmd //c mklink //J "$(cygpath -w "$PWD/$link")" "$(cygpath -w "$src")"
test -f "$link/winrt/Windows.Foundation.h" \
  || { echo "prebuild: junction made but winrt/Windows.Foundation.h missing" >&2; exit 1; }
echo "prebuild: gen/cppwinrt ok"
