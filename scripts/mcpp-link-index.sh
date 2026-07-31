#!/usr/bin/env bash
# Register a `[indices] <name> = { path = ... }` entry with xlings, the way a
# working local checkout ends up registered.
#
# WHY THIS EXISTS. mcpp reads `[indices]` and reports the index correctly —
#     1 index repo configured [sm -> D:/a/SpinningMomo/SpinningMomo/mcpp]
# — but on a Windows CI runner xlings never sees it:
#     E_NOT_FOUND ... searched repos: [xim, mcpplibs]
# The two halves disagree because the handoff is a file, `<project>/.mcpp/
# .xlings.json`, plus a link of the index directory into the project sandbox;
# on a runner neither reliably lands. Writing both explicitly, before mcpp is
# ever invoked, makes the registration deterministic instead of a side effect.
#
# The shape is xlings' own (observed from a working local project):
#     { "index_repos": [ { "name": "sm", "url": "<abs path>" } ], ... }
#
# Usage: mcpp-link-index.sh <project-dir> <index-name> <index-dir>
set -euo pipefail

project=$1
name=$2
index=$3

abs_index=$(cd "$index" && pwd)
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) win_index=$(cygpath -m "$abs_index"); windows=1 ;;
  *)                    win_index=$abs_index;                 windows=0 ;;
esac

mkdir -p "$project/.mcpp"

# ── 1. the registration file xlings actually reads ──────────────────────────
cfg="$project/.mcpp/.xlings.json"
python3 - "$cfg" "$name" "$win_index" <<'PY'
import json, pathlib, sys
cfg, name, url = pathlib.Path(sys.argv[1]), sys.argv[2], sys.argv[3]
data = {}
if cfg.exists():
    try:
        data = json.loads(cfg.read_text())
    except Exception:
        data = {}
repos = [r for r in data.get("index_repos", []) if r.get("name") != name]
repos.insert(0, {"name": name, "url": url})
data["index_repos"] = repos
data.setdefault("lang", "en")
data.setdefault("mirror", "auto")
cfg.write_text(json.dumps(data, indent=2) + "\n")
print(f"link-index: {cfg} -> {url}")
PY

# ── 2. the index directory, linked into both sandbox data dirs ──────────────
# mcpp normally symlinks these. A *directory symlink* on Windows needs
# SeCreateSymbolicLinkPrivilege, which the runner account does not hold, so use
# a junction (no privilege) and fall back to a plain copy — the index is a
# handful of .lua descriptors, duplicating it costs nothing.
for rel in ".mcpp/data" ".mcpp/.xlings/data"; do
  dir="$project/$rel"
  mkdir -p "$dir"
  link="$dir/$name"
  [ -e "$link/index.toml" ] && { echo "link-index: $rel/$name already present"; continue; }
  rm -rf "$link"
  if [ "$windows" = 1 ]; then
    # NOTE: never set MSYS2_ARG_CONV_EXCL='*' around this — it also stops the
    # `//c` -> `/c` and `//J` -> `/J` rewriting, so cmd.exe gets a literal
    # `//c`, does nothing, and reports no error.
    cmd //c mklink //J "$(cygpath -w "$(cd "$dir" && pwd)/$name")" "$(cygpath -w "$abs_index")" || true
  else
    ln -s "$abs_index" "$link" || true
  fi
  if [ ! -f "$link/index.toml" ]; then
    echo "link-index: link unavailable, copying instead"
    rm -rf "$link"
    cp -r "$abs_index" "$link"
  fi
  test -f "$link/index.toml" \
    || { echo "link-index: $rel/$name has no index.toml" >&2; exit 1; }
  echo "link-index: $rel/$name -> $abs_index"
done
