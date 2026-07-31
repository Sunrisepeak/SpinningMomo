#!/usr/bin/env bash
# Pre-flight every mcpp project that can be judged from Linux before spending a
# CI round-trip on it.
#
# mcpp's `[toolchain] windows = ...` only binds on Windows hosts, so on Linux
# the same manifests build with the default gcc. That covers manifest schema,
# module graph, `import std`, and template-heavy code — everything except the
# genuinely Windows-only facts (MSVC dialect, Win32/WinRT headers, rc.exe).
# Those are marked `# windows-only` and are skipped with a note rather than
# reported as failures.
set -uo pipefail
cd "$(dirname "$0")/.."

fail=0
for manifest in probes/*/mcpp.toml "${@:-}"; do
  [ -f "$manifest" ] || continue
  dir=$(dirname "$manifest")
  if grep -qriE '#include <(windows|winrt/|d3d11|dwmapi|dcomp)' "$dir" 2>/dev/null; then
    echo "SKIP  $dir  (windows-only — CI is the only judge)"
    continue
  fi
  if out=$(cd "$dir" && mcpp build 2>&1); then
    echo "PASS  $dir"
  else
    echo "FAIL  $dir"
    echo "$out" | tail -20 | sed 's/^/      /'
    fail=1
  fi
done
exit $fail
