set -euo pipefail
exe=$(find target -name 'p2rc.exe' | head -1)
test -n "$exe" || { echo "FAIL: no exe produced"; exit 1; }
ver=$(powershell -NoProfile -Command \
  "(Get-Item '$(cygpath -w "$exe")').VersionInfo.ProductVersion" | tr -d '\r')
echo "embedded ProductVersion = '$ver'"
case "$ver" in
  9.9.9.0*) echo "p2: version resource survived into the PE" ;;
  *) echo "FAIL: expected 9.9.9.0, got '$ver'"; exit 1 ;;
esac
