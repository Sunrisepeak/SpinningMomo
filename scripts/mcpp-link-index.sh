#!/usr/bin/env bash
# Register the project's path index with the xlings sandbox mcpp actually
# consults.
#
# WHY THIS EXISTS. Declaring `[indices] sm = { path = "mcpp" }` in mcpp.toml is
# enough for mcpp — it reports
#     1 index repo configured [sm -> D:/a/SpinningMomo/SpinningMomo/mcpp]
# — but on a fresh machine xlings never sees it:
#     E_NOT_FOUND ... searched repos: [xim, mcpplibs]
#
# The two halves disagree because of WHERE the registration lives. mcpp
# materialises the manifest's `[indices]` into `<project>/.mcpp/.xlings.json`,
# but the sandbox xlings that resolves packages runs with
# `[xlings] home = ""` (see ~/.mcpp/config.toml), i.e. XLINGS_HOME =
# $MCPP_HOME/registry — so the file it reads is
# `~/.mcpp/registry/.xlings.json`, not the project one. A long-lived local
# checkout has the index in there from some earlier run and works; a CI runner
# does not and never will, which is exactly the "works locally, fails in CI"
# split this script closes.
#
# `mcpp index add <name> <path>` is the supported way to write that file.
#
# Usage: mcpp-link-index.sh <project-dir> <index-name> <index-dir>
set -euo pipefail

project=$1
name=$2
index=$3

abs_index=$(cd "$index" && pwd)
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) reg_index=$(cygpath -m "$abs_index") ;;
  *)                    reg_index=$abs_index ;;
esac

MCPP="${MCPP:-mcpp}"

# Adding an index that is already registered is not an error worth stopping for.
"$MCPP" index add "$name" "$reg_index" || echo "link-index: index add reported a failure (may already exist)"

# Assert the outcome instead of trusting it: the failure mode this script exists
# to close was silent.
sandbox_cfg="${MCPP_HOME:-$HOME/.mcpp}/registry/.xlings.json"
[ -f "$sandbox_cfg" ] || sandbox_cfg="${USERPROFILE:-$HOME}/.mcpp/registry/.xlings.json"
if [ -f "$sandbox_cfg" ]; then
  python3 - "$sandbox_cfg" "$name" "$reg_index" <<'PY'
import json, pathlib, sys
cfg, name, url = pathlib.Path(sys.argv[1]), sys.argv[2], sys.argv[3]
data = json.loads(cfg.read_text())
repos = data.get("index_repos", [])
if any(r.get("name") == name for r in repos):
    print(f"link-index: '{name}' registered in {cfg}")
    raise SystemExit(0)
# `mcpp index add` did not take — write the entry ourselves rather than let the
# build fail 200 lines later with a lookup error that names neither cause.
repos.append({"name": name, "url": url})
data["index_repos"] = repos
cfg.write_text(json.dumps(data))
print(f"link-index: wrote '{name}' into {cfg} directly")
PY
else
  echo "link-index: no sandbox .xlings.json found; mcpp has not initialised its registry" >&2
  exit 1
fi
