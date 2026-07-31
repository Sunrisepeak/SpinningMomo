#!/usr/bin/env bash
# Everything the mcpp build needs that mcpp itself cannot produce.
#
# All three items belong in a `build.mcpp` (mcpp's Cargo-build.rs equivalent).
# That is not usable on Windows today: mcpp compiles build.mcpp by handing the
# compiler path to cmd.exe unquoted (build_program.cppm:699 -> capture_exec),
# and every MSVC path contains a space, so it fails with
#     'C:\Program' is not recognized as an internal or external command
# before build.mcpp's own code runs. Until that is fixed upstream this script
# is the pre-build step and mcpp.toml only names the artefacts it produces.
#
#   gen/app.res              resources/app.rc compiled by rc.exe
#   gen/cppwinrt/            C++/WinRT projection headers
#   gen/webview2/            WebView2 SDK headers + static loader
set -euo pipefail
cd "$(dirname "$0")/.."

# MSYS rewrites anything that looks like a POSIX path, so a bare `/nologo`
# reaches a Windows tool as C:/Program Files/Git/nologo.
export MSYS2_ARG_CONV_EXCL='*'

mkdir -p gen

kit=""
for root in "/c/Program Files (x86)/Windows Kits/10" "/c/Program Files/Windows Kits/10"; do
  [ -d "$root/Include" ] && kit="$root" && break
done
[ -n "$kit" ] || { echo "prebuild: no Windows Kit found" >&2; exit 1; }

# ── 1. resources/app.rc -> gen/app.res ──────────────────────────────────────
if [ resources/app.rc -nt gen/app.res ] || [ ! -f gen/app.res ]; then
  rc=$(find "$kit/bin" -maxdepth 3 -name rc.exe -path '*/x64/*' 2>/dev/null | sort -V | tail -1)
  [ -n "$rc" ] || { echo "prebuild: rc.exe not found under $kit/bin" >&2; exit 1; }
  # rc.exe resolves #include through INCLUDE, which nothing sets here — mcpp
  # synthesises INCLUDE for cl.exe only, and app.rc starts with <windows.h>.
  ver=$(basename "$(dirname "$(dirname "$rc")")")
  inc="$kit/Include/$ver"
  INCLUDE="$(cygpath -w "$inc/um");$(cygpath -w "$inc/shared");$(cygpath -w "$inc/ucrt")" \
    "$rc" /nologo /fo "$(cygpath -w "$PWD/gen/app.res")" "$(cygpath -w "$PWD/resources/app.rc")"
  echo "prebuild: gen/app.res"
else
  echo "prebuild: gen/app.res up to date"
fi

# ── 2. C++/WinRT projection -> gen/cppwinrt ─────────────────────────────────
# mcpp's synthesised INCLUDE covers {ucrt,um,shared,winrt} only
# (toolchain/msvc.cppm:457-460); the `winrt` dir there is the ABI header set
# (lowercase windows.foundation.h), not the C++ projection this project uses.
if [ ! -f gen/cppwinrt/winrt/Windows.Foundation.h ]; then
  found=$(find "$kit/Include" -maxdepth 4 -name 'Windows.Foundation.h' -path '*/winrt/*' 2>/dev/null | sort -V | tail -1)
  if [ -n "$found" ]; then
    root=$(dirname "$(dirname "$found")")
    echo "prebuild: linking prebuilt projection from $root"
    MSYS2_ARG_CONV_EXCL= cmd //c mklink //J "$(cygpath -w "$PWD/gen/cppwinrt")" "$(cygpath -w "$root")"
  else
    cppwinrt=$(find "$kit/bin" -maxdepth 3 -name cppwinrt.exe 2>/dev/null | sort -V | tail -1)
    [ -n "$cppwinrt" ] || { echo "prebuild: neither a projection nor cppwinrt.exe" >&2; exit 1; }
    echo "prebuild: generating projection with $cppwinrt"
    mkdir -p gen/cppwinrt
    "$cppwinrt" -in local -out "$(cygpath -w "$PWD/gen/cppwinrt")"
  fi
  test -f gen/cppwinrt/winrt/Windows.Foundation.h \
    || { echo "prebuild: projection still missing winrt/Windows.Foundation.h" >&2; exit 1; }
  echo "prebuild: gen/cppwinrt"
else
  echo "prebuild: gen/cppwinrt up to date"
fi

# ── 3. WebView2 SDK -> gen/webview2 ─────────────────────────────────────────
# Distributed only as a NuGet package (headers + a prebuilt static loader), so
# it is not a source package and does not belong in the index. Unpacked here
# and reached through project-relative include_dirs / ldflags.
WEBVIEW2_VERSION="${WEBVIEW2_VERSION:-1.0.3485.44}"
if [ ! -f gen/webview2/include/WebView2.h ]; then
  echo "prebuild: fetching WebView2 $WEBVIEW2_VERSION"
  tmp=$(mktemp -d)
  curl -fsSL -o "$tmp/wv2.zip" \
    "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/${WEBVIEW2_VERSION}/microsoft.web.webview2.${WEBVIEW2_VERSION}.nupkg"
  (cd "$tmp" && unzip -qo wv2.zip -d unpacked)
  mkdir -p gen/webview2/include gen/webview2/lib
  cp "$tmp/unpacked/build/native/include/"*.h gen/webview2/include/
  cp "$tmp/unpacked/build/native/x64/WebView2LoaderStatic.lib" gen/webview2/lib/
  rm -rf "$tmp"
  test -f gen/webview2/include/WebView2.h
  echo "prebuild: gen/webview2"
else
  echo "prebuild: gen/webview2 up to date"
fi

echo "prebuild: done"
