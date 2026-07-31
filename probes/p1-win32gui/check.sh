set -euo pipefail
exe=$(find target -name 'p1win32gui.exe' | head -1)
test -n "$exe" || { echo "FAIL: no exe produced"; exit 1; }
echo "produced: $exe"
"$exe" && echo "p1: GUI exe ran and exited 0"
