#!/usr/bin/env bash
# Compile app.rc -> gen/app.res before `mcpp build`.
#
# This belongs in a `build.mcpp` (mcpp's Cargo-build.rs equivalent) and started
# there. It cannot work on Windows today: mcpp compiles build.mcpp by handing
# the compiler path to cmd.exe unquoted (build_program.cppm:699 ->
# capture_exec), and every MSVC path contains a space, so it dies with
#     'C:\Program' is not recognized as an internal or external command
# before build.mcpp's own code runs at all. Until that is fixed upstream the
# resource step lives outside mcpp and the manifest only names the .res.
set -euo pipefail
cd "$(dirname "$0")"

# Two Windows-isms, both load-bearing:
#  1. MSYS rewrites any argument that looks like a POSIX path, so a bare
#     `/nologo` reaches rc.exe as `C:/Program Files/Git/nologo`.
#  2. rc.exe reads `/` as the option introducer, so a forward-slash output
#     path is parsed as a switch (RC1107). Both arguments must be Windows form.
export MSYS2_ARG_CONV_EXCL='*'

rc=""
for root in "/c/Program Files (x86)/Windows Kits/10/bin" "/c/Program Files/Windows Kits/10/bin"; do
  [ -d "$root" ] || continue
  cand=$(find "$root" -maxdepth 3 -name rc.exe -path '*/x64/*' 2>/dev/null | sort -V | tail -1)
  [ -n "$cand" ] && rc="$cand" && break
done
[ -n "$rc" ] || { echo "prebuild: rc.exe not found under any Windows Kit" >&2; exit 1; }
echo "prebuild: using $rc"

mkdir -p gen
"$rc" /nologo /fo "$(cygpath -w "$PWD/gen/app.res")" "$(cygpath -w "$PWD/app.rc")"
test -f gen/app.res || { echo "prebuild: rc.exe produced no gen/app.res" >&2; exit 1; }
echo "prebuild: gen/app.res ok"
