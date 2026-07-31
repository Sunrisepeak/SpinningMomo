#!/usr/bin/env bash
# Pre-create the links mcpp uses to register a `[indices] <name> = { path = ... }`
# entry, as NTFS junctions.
#
# WHY. mcpp registers a path index by symlinking it into the project sandbox:
#     <project>/.mcpp/data/<name>          -> <index dir>
#     <project>/.mcpp/.xlings/data/<name>  -> <index dir>
# On Windows a *directory symlink* needs SeCreateSymbolicLinkPrivilege, which
# the GitHub runner account does not hold (Developer Mode off). The creation
# fails silently, so xlings is never told about the index and every package in
# it misses — the symptom is mcpp reporting
#     1 index repo configured [sm -> D:/.../mcpp]
# while xlings reports
#     E_NOT_FOUND ... searched repos: [xim, mcpplibs]
# which reads like a lookup bug and is really a filesystem-permission one.
#
# A junction (`mklink /J`) points at a directory with no privilege requirement
# and is transparent to every reader. Creating it before mcpp runs means mcpp
# finds the link already in place.
#
# Usage: mcpp-link-index.sh <project-dir> <index-name> <index-dir>
set -euo pipefail

project=$1
name=$2
index=$3

case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) ;;
  *) echo "link-index: not Windows, mcpp's own symlink works — nothing to do"; exit 0 ;;
esac

export MSYS2_ARG_CONV_EXCL='*'
abs_index=$(cd "$index" && pwd)

for rel in ".mcpp/data" ".mcpp/.xlings/data"; do
  dir="$project/$rel"
  mkdir -p "$dir"
  link="$dir/$name"
  if [ -e "$link" ]; then
    echo "link-index: $rel/$name already present"
    continue
  fi
  cmd //c mklink //J "$(cygpath -w "$(cd "$dir" && pwd)/$name")" "$(cygpath -w "$abs_index")" >/dev/null
  test -f "$link/index.toml" \
    || { echo "link-index: junction $rel/$name does not expose index.toml" >&2; exit 1; }
  echo "link-index: $rel/$name -> $abs_index"
done
