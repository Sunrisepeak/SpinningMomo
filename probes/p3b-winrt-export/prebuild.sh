#!/usr/bin/env bash
# Expose the C++/WinRT projection headers (winrt/Windows.*.h) under a
# space-free, version-independent path inside the project.
#
# Two mcpp gaps make this necessary:
#  1. The synthesised INCLUDE covers {ucrt,um,shared,winrt} only
#     (src/toolchain/msvc.cppm:457-460). The `winrt` dir there is the ABI
#     header set (lowercase windows.foundation.h), NOT the C++ projection.
#  2. An include_dirs entry containing spaces is emitted unquoted, so
#     "C:/Program Files (x86)/..." reaches cl.exe as three arguments and the
#     tail lands in the source-file position.
#
# The projection is not always present as headers: some SDK layouts ship only
# cppwinrt.exe and expect the consumer to GENERATE the projection. Handle both.
set -euo pipefail
cd "$(dirname "$0")"

export MSYS2_ARG_CONV_EXCL='*'
out="gen/cppwinrt"
[ -f "$out/winrt/Windows.Foundation.h" ] && { echo "prebuild: already present"; exit 0; }
mkdir -p gen

kit=""
for root in "/c/Program Files (x86)/Windows Kits/10" "/c/Program Files/Windows Kits/10"; do
  [ -d "$root/Include" ] && kit="$root" && break
done
[ -n "$kit" ] || { echo "prebuild: no Windows Kit" >&2; exit 1; }

echo "prebuild: surveying $kit for a prebuilt projection"
found=$(find "$kit/Include" -maxdepth 4 -name 'Windows.Foundation.h' -path '*/winrt/*' 2>/dev/null | sort -V | tail -1)
if [ -n "$found" ]; then
  # .../Include/<ver>/cppwinrt/winrt/Windows.Foundation.h -> link the cppwinrt root
  root=$(dirname "$(dirname "$found")")
  echo "prebuild: prebuilt projection at $root"
  MSYS2_ARG_CONV_EXCL= cmd //c mklink //J "$(cygpath -w "$PWD/$out")" "$(cygpath -w "$root")"
else
  echo "prebuild: no prebuilt projection; generating with cppwinrt.exe"
  cppwinrt=$(find "$kit/bin" -maxdepth 3 -name cppwinrt.exe 2>/dev/null | sort -V | tail -1)
  [ -n "$cppwinrt" ] || { echo "prebuild: cppwinrt.exe not found either" >&2
                          find "$kit/Include" -maxdepth 2 -type d | head -20 >&2; exit 1; }
  echo "prebuild: using $cppwinrt"
  mkdir -p "$out"
  "$cppwinrt" -in local -out "$(cygpath -w "$PWD/$out")"
fi

test -f "$out/winrt/Windows.Foundation.h" || {
  echo "prebuild: still no winrt/Windows.Foundation.h; what we have:" >&2
  find "$out" -maxdepth 2 2>/dev/null | head -20 >&2
  exit 1; }
echo "prebuild: $out ok"
