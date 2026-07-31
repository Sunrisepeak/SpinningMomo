#!/usr/bin/env python3
"""Convert a project header (+ its implementation) into a C++23 named module.

The transform is mechanical because the codebase already satisfies the two
preconditions that normally make modularisation hard:

  * external `<>` includes appear ONLY under src/vendor/, so the global module
    fragment has exactly one kind of entry (a vendor facade), and
  * every header is self-contained without the PCH, so nothing has to be added.

So the edit per file is confined to the top of it:

    #pragma once                     ->  module;
    #include "vendor/windows.hpp"        #include "vendor/windows.hpp"   (kept, in the GMF)
    #include "vendor/std.hpp"        ->  export module utils.timer.timeout;
    #include "utils/logger/logger.hpp"   import std;
    namespace utils::timeout {          import utils.logger.logger;
                                        export namespace utils::timeout {

Vendor facades stay plain headers and are pulled into each module's GMF. That is
the strategy probe p3c validated; entities in a GMF attach to the global module,
so two modules including the same facade share one entity and there is no ODR
problem. The cost is re-parsing the SDK header per TU — exactly what the PCH
amortises today, and what BMIs will amortise once enough of the tree is modules.

Usage:  mcpp-modularize.py <header path relative to src/, without .hpp> ...
        mcpp-modularize.py --rewrite-consumers <module path> ...
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"\s*$')
PRAGMA_ONCE_RE = re.compile(r'^\s*#\s*pragma\s+once\s*$')
NAMESPACE_RE = re.compile(r'^namespace\s+([A-Za-z_][A-Za-z0-9_:]*)\s*\{')


# mcpp forbids a handful of top-level module names to avoid collisions between
# packages: core, util, common, std, detail, internal, base
# (modgraph/validate.cppm:48). `core/...` is exactly what this project's tree is
# built around, so every module carries the project's namespace prefix — the
# same `sm` the package index uses.
MODULE_PREFIX = "sm."


def module_name(rel: str) -> str:
    """src-relative path without extension -> dotted module name."""
    return MODULE_PREFIX + rel.replace("/", ".")


def split_top(lines: list[str], modules: set[str]) -> tuple[list[str], list[str], int]:
    """Split the leading include block three ways and return where real content
    starts.

    A module unit may not `#include` in its purview, so anything that stays an
    include has to move into the global module fragment. `modules` is the set of
    src-relative paths that ARE modules by the end of this run; a project header
    outside that set is still a header and keeps its `#include` — in the GMF.
    That mixing is the whole reason an incremental migration is possible."""
    gmf: list[str] = []
    imports: list[str] = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if PRAGMA_ONCE_RE.match(line) or not line.strip():
            i += 1
            continue
        m = INCLUDE_RE.match(line)
        if not m:
            break
        target = m.group(1)
        if target == "vendor/std.hpp":
            pass  # becomes `import std;`
        elif target.startswith("vendor/"):
            gmf.append(line.rstrip())
        elif target.removesuffix(".hpp") in modules:
            imports.append(target.removesuffix(".hpp"))
        else:
            gmf.append(line.rstrip())
        i += 1
    return gmf, imports, i


def convert_header(rel: str, modules: set[str]) -> Path:
    src = SRC / f"{rel}.hpp"
    text = src.read_text(encoding="utf-8")
    lines = text.splitlines()
    gmf, imports, start = split_top(lines, modules)
    body = lines[start:]

    # Export the top-level namespace. Nothing below it needs touching: `export`
    # on the namespace covers every declaration inside it.
    for n, line in enumerate(body):
        if NAMESPACE_RE.match(line):
            body[n] = "export " + line
            break

    out: list[str] = ["module;", ""]
    out += gmf
    if gmf:
        out.append("")
    out.append(f"export module {module_name(rel)};")
    out.append("")
    out.append("import std;")
    for p in imports:
        out.append(f"import {module_name(p)};")
    out.append("")
    out += body

    dst = SRC / f"{rel}.cppm"
    dst.write_text("\n".join(out).rstrip() + "\n", encoding="utf-8")
    src.unlink()
    convert_header.last_gmf = gmf  # handed to convert_impl
    return dst


def convert_impl(rel: str, modules: set[str], iface_gmf: list[str] | None = None) -> Path | None:
    """Turn x.cpp into the module implementation unit for x.

    `iface_gmf` is the interface unit's global module fragment, and it MUST be
    replayed here. Entities a module interface pulls in through a GMF
    `#include` attach to the GLOBAL module, not to the named module, so they
    are invisible in the implementation unit even though it implicitly imports
    its own interface. Before modules, `encoder.cpp` saw `VideoCodec` because
    it self-included `encoder.hpp` which included `types.hpp`; drop the
    self-include and that chain is gone."""
    src = SRC / f"{rel}.cpp"
    if not src.exists():
        return None
    lines = src.read_text(encoding="utf-8").splitlines()

    # The self-include is what identifies the pair; drop it.
    self_inc = f"{rel}.hpp"
    lines = [l for l in lines if not (INCLUDE_RE.match(l) and INCLUDE_RE.match(l).group(1) == self_inc)]

    gmf, imports, start = split_top(lines, modules)
    body = lines[start:]

    # Replay the interface's GMF first, then this unit's own, de-duplicated and
    # order-preserving.
    merged: list[str] = []
    for line in (iface_gmf or []) + gmf:
        if line not in merged:
            merged.append(line)

    out: list[str] = ["module;", ""]
    out += merged
    if merged:
        out.append("")
    out.append(f"module {module_name(rel)};")
    out.append("")
    out.append("import std;")
    for p in imports:
        out.append(f"import {module_name(p)};")
    out.append("")
    out += body

    src.write_text("\n".join(out).rstrip() + "\n", encoding="utf-8")
    return src


def rewrite_consumers(rels: list[str]) -> int:
    """Every `#include "<rel>.hpp"` anywhere in src/ becomes `import <mod>;`."""
    wanted = {f"{r}.hpp": module_name(r) for r in rels}
    touched = 0
    for path in list(SRC.rglob("*.hpp")) + list(SRC.rglob("*.cpp")) + list(SRC.rglob("*.cppm")):
        text = path.read_text(encoding="utf-8")
        new_lines = []
        changed = False
        for line in text.splitlines():
            m = INCLUDE_RE.match(line)
            if m and m.group(1) in wanted:
                new_lines.append(f"import {wanted[m.group(1)]};")
                changed = True
            else:
                new_lines.append(line)
        if changed:
            path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")
            touched += 1
    return touched


def main() -> int:
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 2
    if args[0] == "--rewrite-consumers":
        n = rewrite_consumers(args[1:])
        print(f"rewrote includes in {n} files")
        return 0
    modules = set(args)
    for rel in args:
        h = convert_header(rel, modules)
        i = convert_impl(rel, modules, convert_header.last_gmf)
        print(f"{rel}: {h.relative_to(ROOT)}" + (f" + {i.relative_to(ROOT)}" if i else " (header-only)"))
    rewrite_consumers(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
