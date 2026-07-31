#!/usr/bin/env bash
# Compile app.rc -> gen/app.res before `mcpp build`.
#
# This was originally a `build.mcpp` (mcpp's Cargo-build.rs equivalent), which
# is the architecturally right home for it. It does not work on Windows today:
# mcpp compiles build.mcpp by handing the compiler path to cmd.exe unquoted
# (src/build/build_program.cppm:699 -> capture_exec), and every MSVC path
# contains a space, so it dies with
#     'C:\Program' is not recognized as an internal or external command
# before build.mcpp's own code ever runs. Upstream bug; until it is fixed, the
# resource step lives outside mcpp and only the .res path is declared in the
# manifest's ldflags.
set -euo pipefail
cd "$(dirname "$0")"

rc=""
for root in "/c/Program Files (x86)/Windows Kits/10/bin" "/c/Program Files/Windows Kits/10/bin"; do
  [ -d "$root" ] || continue
  # Newest versioned bin wins; `sort -V` so 10.0.26100 beats 10.0.9.
  cand=$(find "$root" -maxdepth 3 -name rc.exe -path '*/x64/*' 2>/dev/null | sort -V | tail -1)
  [ -n "$cand" ] && rc="$cand" && break
done
[ -n "$rc" ] || { echo "prebuild: rc.exe not found under any Windows Kit" >&2; exit 1; }
echo "prebuild: using $rc"

mkdir -p gen
"$rc" /nologo /fo gen/app.res app.rc
test -f gen/app.res || { echo "prebuild: rc.exe produced no gen/app.res" >&2; exit 1; }
echo "prebuild: gen/app.res ok"
