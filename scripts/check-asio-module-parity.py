#!/usr/bin/env python3
"""Keep the xmake build's `asio` module identical to the index package's.

The project consumes Asio as a module under BOTH build systems, but the module
unit has two different sources:

  mcpp   the index package `chriskohlhoff.asio` generates it, from the
         descriptor's `generated_files["mcpp_generated/asio.cppm"]`
  xmake  vcpkg ships headers only, so `third_party/asio-module/asio.cppm`
         carries the same wrapper in-tree

Two copies of one interface drift, and the drift is invisible until one build
fails on a symbol the other exports. This script is the thing that makes it
visible: it extracts the wrapper from the descriptor and diffs it against the
mirror.

    check-asio-module-parity.py                  # verify (exit 1 on drift)
    check-asio-module-parity.py --write          # regenerate the mirror

The comparison target is whichever descriptor the MCPP BUILD ACTUALLY CONSUMES,
in this order:

  1. --descriptor PATH
  2. the installed chriskohlhoff.asio payload under the mcpp registry — what the
     build actually consumed
  3. a sibling checkout of mcpp-index

A caveat worth knowing on a developer machine: a descriptor can change WITHOUT
its version changing (an export-surface amendment is exactly that), and an
already-installed payload is not invalidated by it. If (2) looks stale, clear it:

    rm -rf ~/.mcpp/registry/data/xpkgs/chriskohlhoff-x-asio

Skips (exit 0) when no descriptor can be found — on a machine that has never run
`mcpp build` there is nothing to compare against, and failing there would only
teach people to ignore this check.
"""

from __future__ import annotations

import argparse
import difflib
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MIRROR = ROOT / "third_party" / "asio-module" / "asio.cppm"

# The mirror carries a header explaining what it is; the shared body starts at
# this marker. Everything above it is local commentary and is not compared.
MARKER = "// " + "-" * 77

WRAPPER_RE = re.compile(
    r'\["mcpp_generated/asio\.cppm"\]\s*=\s*\[==\[\n(.*?)\n\]==\]', re.S
)


def candidate_descriptors() -> list[Path]:
    out: list[Path] = []

    registry = Path(os.environ.get("MCPP_HOME", Path.home() / ".mcpp")) / "registry"
    # The package payload keeps the descriptor it was installed from; the index
    # checkouts below are only a fallback for a machine that has not built yet.
    for pat in ("data/xpkgs/chriskohlhoff-x-asio/**/*.lua",):
        out.extend(sorted(registry.glob(pat)))

    for repo in (
        ROOT.parent / "mcpp-index",
        Path.home() / "workspace" / "github" / "mcpplibs" / "mcpp-index",
    ):
        p = repo / "pkgs" / "c" / "chriskohlhoff.asio.lua"
        if p.is_file():
            out.append(p)

    return out


def extract(descriptor: Path) -> str | None:
    m = WRAPPER_RE.search(descriptor.read_text(encoding="utf-8", errors="replace"))
    return m.group(1) if m else None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true", help="regenerate the mirror")
    ap.add_argument("--descriptor", type=Path, help="path to chriskohlhoff.asio.lua")
    args = ap.parse_args()

    sources = [args.descriptor] if args.descriptor else candidate_descriptors()
    upstream = None
    used = None
    for d in sources:
        if d and d.is_file():
            body = extract(d)
            if body is not None:
                upstream, used = body, d
                break

    if upstream is None:
        print("asio-parity: SKIP — no chriskohlhoff.asio descriptor found "
              "(nothing to compare against on this machine)")
        return 0

    if not MIRROR.is_file():
        print(f"asio-parity: FAIL — mirror missing: {MIRROR}")
        return 1

    text = MIRROR.read_text(encoding="utf-8")
    if MARKER not in text:
        print(f"asio-parity: FAIL — mirror lost its header marker: {MIRROR}")
        return 1
    header, mirror_body = text.split(MARKER, 1)
    mirror_body = mirror_body.lstrip("\n").rstrip("\n")

    if mirror_body == upstream.rstrip("\n"):
        print(f"asio-parity: OK — mirror matches {used}")
        return 0

    if args.write:
        MIRROR.write_text(header + MARKER + "\n\n" + upstream.rstrip("\n") + "\n",
                          encoding="utf-8")
        print(f"asio-parity: rewrote mirror from {used}")
        return 0

    print(f"asio-parity: FAIL — the xmake mirror and {used} have drifted.\n"
          f"  The descriptor is the source of truth; regenerate with\n"
          f"    python3 scripts/check-asio-module-parity.py --write\n")
    for line in difflib.unified_diff(
        mirror_body.splitlines(), upstream.rstrip("\n").splitlines(),
        fromfile=str(MIRROR.relative_to(ROOT)), tofile=str(used), lineterm="",
    ):
        print("  " + line)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
