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

# MSYS rewrites anything that looks like a POSIX path, so a bare `/nologo`
# reaches rc.exe as C:/Program Files/Git/nologo. rc.exe also reads `/` as the
# option introducer, so every path argument must be Windows form.
export MSYS2_ARG_CONV_EXCL='*'

kit=""
for root in "/c/Program Files (x86)/Windows Kits/10" "/c/Program Files/Windows Kits/10"; do
  [ -d "$root/bin" ] && kit="$root" && break
done
[ -n "$kit" ] || { echo "prebuild: no Windows Kit" >&2; exit 1; }

rc=$(find "$kit/bin" -maxdepth 3 -name rc.exe -path '*/x64/*' 2>/dev/null | sort -V | tail -1)
[ -n "$rc" ] || { echo "prebuild: rc.exe not found under $kit/bin" >&2; exit 1; }
echo "prebuild: using $rc"

# rc.exe resolves #include through INCLUDE, which nothing has set here — mcpp
# synthesises INCLUDE for cl.exe only, and app.rc starts with <windows.h>.
# Derive it from the same SDK version the rc.exe binary came from.
ver=$(basename "$(dirname "$(dirname "$rc")")")
inc="$kit/Include/$ver"
[ -d "$inc/um" ] || { echo "prebuild: no Include/$ver/um under $kit" >&2; exit 1; }
export INCLUDE="$(cygpath -w "$inc/um");$(cygpath -w "$inc/shared");$(cygpath -w "$inc/ucrt")"
echo "prebuild: INCLUDE=$INCLUDE"

mkdir -p gen
"$rc" /nologo /fo "$(cygpath -w "$PWD/gen/app.res")" "$(cygpath -w "$PWD/app.rc")"
test -f gen/app.res || { echo "prebuild: rc.exe produced no gen/app.res" >&2; exit 1; }
echo "prebuild: gen/app.res ok"
