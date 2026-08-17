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
        mcpp-modularize.py --closure <seed header, src-relative WITH .hpp> ...
        mcpp-modularize.py --normalize        (also qualifies C types and orders imports)
        mcpp-modularize.py --rename-prefix <old top-level module segment> ...
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

# Vendor facades that are no longer headers: the library itself is consumed as a
# module, so `#include "vendor/x.hpp"` becomes `import <module>;` everywhere —
# in a module's purview AND in a plain .cpp, which may import without being a
# module unit itself.
#
# asio is the first and the reason the rule exists. While it lived in each
# module's global module fragment, its template specializations
# (`service_registry::use_service<config_service>` and friends) were
# re-instantiated in every importing TU and MSVC could not reconcile them:
#   fatal error C1116: unrecoverable error importing module 'sm.core.rpc.state'
# A real module instantiates them ONCE, inside asio.
#
# The consequence is that a facade listed here can no longer be reached from an
# unconverted header — a header cannot `import`. So every .hpp that includes one
# must be in the same conversion batch. `--closure` computes that set.
VENDOR_MODULES = {
    # The library itself is a module package — no facade at all, because the
    # module IS the interface.
    "vendor/asio.hpp": "asio",

    # Facades that became modules in place. These still exist as the single
    # place the project names an external library; what changed is that the
    # third-party header is parsed ONCE instead of once per consumer.
    "vendor/xxhash.hpp": "sm.vendor.xxhash",
    "vendor/webp.hpp":   "sm.vendor.webp",
    "vendor/dkm.hpp":    "sm.vendor.dkm",
    "vendor/sqlite.hpp": "sm.vendor.sqlite",
    "vendor/spdlog.hpp": "sm.vendor.spdlog",
    "vendor/rfl.hpp":    "sm.vendor.rfl",
    "vendor/uwebsockets.hpp": "sm.vendor.uwebsockets",
}


# A module name is a dot-separated sequence of IDENTIFIERS, so no component may
# be a keyword. The tree has one such directory name today — `core/http_server/
# static.cpp` would map to `sm.core.http_server.static`, which clang rejects
# with `expected a module name after 'module'`. The rule is a suffix rather than
# a rename so it is derivable from the path in both directions.
CXX_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool",
    "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class",
    "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast",
    "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete",
    "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
    "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
    "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
    "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
    "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
    "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}


def module_name(rel: str) -> str:
    """src-relative path without extension -> dotted module name."""
    parts = [f"{p}_" if p in CXX_KEYWORDS else p for p in rel.split("/")]
    return MODULE_PREFIX + ".".join(parts)


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
        elif target in VENDOR_MODULES:
            imports.append(("raw", VENDOR_MODULES[target]))
        elif target.startswith("vendor/"):
            gmf.append(line.rstrip())
        elif target.removesuffix(".hpp") in modules:
            imports.append(("project", target.removesuffix(".hpp")))
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
    for kind, p in imports:
        out.append(f"import {p};" if kind == "raw" else f"import {module_name(p)};")
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
    for kind, p in imports:
        out.append(f"import {p};" if kind == "raw" else f"import {module_name(p)};")
    out.append("")
    out += body

    src.write_text("\n".join(out).rstrip() + "\n", encoding="utf-8")
    return src


def rewrite_consumers(rels: list[str]) -> int:
    """Every `#include "<rel>.hpp"` anywhere in src/ becomes `import <mod>;`."""
    wanted = {f"{r}.hpp": module_name(r) for r in rels}
    wanted.update(VENDOR_MODULES)
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


MODULE_DECL_RE = re.compile(r'^(?:export )?module\s+[A-Za-z_]')
IMPORT_RE = re.compile(r'^\s*import\s')


C_TYPES = ("size_t", "ptrdiff_t", "intptr_t", "uintptr_t",
           "int8_t", "int16_t", "int32_t", "int64_t",
           "uint8_t", "uint16_t", "uint32_t", "uint64_t")
C_TYPE_RE = re.compile(r"(?<![\w:.>])(" + "|".join(C_TYPES) + r")\b")


def _mask_comments_and_strings(text: str) -> bytearray:
    """Positions that are inside a comment or a string/char literal."""
    mask = bytearray(len(text))
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif c in "\"'":
            j = i + 1
            while j < n:
                if text[j] == "\\":
                    j += 2
                    continue
                if text[j] == c:
                    j += 1
                    break
                j += 1
        else:
            i += 1
            continue
        for k in range(i, min(j, n)):
            mask[k] = 1
        i = j
    return mask


def order_imports() -> tuple[int, list[str]]:
    """In a PLAIN translation unit, put every `#include` before every `import`.

    Both orders are legal C++. Only one of them compiles here.

    A TU that imports a module whose global module fragment included
    <windows.h>, and then ALSO includes <windows.h> textually, ends up with two
    parses of the SDK. Under clang the merge fails on winuser.h's unnamed
    structs — `typedef struct {…} FLASHWINFO, *PFLASHWINFO;` and BSMINFO — and
    every function taking one becomes a redeclaration with a different
    signature:

        winuser.h:4698: error: conflicting types for 'FlashWindowEx'
        winuser.h:4698: note: previous declaration is here     <- the same line

    Seeing the textual declarations FIRST and merging the BMI into them is a
    different path inside clang than the reverse, and only that direction works.
    probes/p8-asio-windows is the two rounds of that experiment: import-first
    fails, include-first passes, nothing else changed.

    Module units need no help — their includes are in the global module fragment
    and their imports in the purview, which is already this order.

    Returns (files changed, files skipped-with-reason).
    """
    changed = 0
    skipped: list[str] = []
    for path in SRC.rglob("*.cpp"):
        lines = path.read_text(encoding="utf-8").splitlines()
        if any(MODULE_DECL_RE.match(l) for l in lines):
            continue  # module unit: GMF/purview already orders it

        # The prologue is everything before the first line of real code.
        end = len(lines)
        for i, l in enumerate(lines):
            s = l.strip()
            if not s or s.startswith(("//", "/*", "*", "#include", "import ")):
                continue
            end = i
            break
        prologue, rest = lines[:end], lines[end:]

        imports = [i for i, l in enumerate(prologue) if IMPORT_RE.match(l)]
        includes = [i for i, l in enumerate(prologue) if l.lstrip().startswith("#include")]
        if not imports or not includes:
            continue
        if max(includes) < min(imports):
            continue  # already ordered

        # A conditional in the prologue means the order is not ours to decide.
        if any(l.lstrip().startswith(("#if", "#el", "#endif", "#define", "#undef"))
               for l in prologue):
            skipped.append(str(path.relative_to(ROOT)))
            continue

        import_lines = [prologue[i] for i in imports]
        kept = [l for i, l in enumerate(prologue) if i not in set(imports)]
        # Removing an import can leave two blank lines where it used to be.
        collapsed: list[str] = []
        for l in kept:
            if not l.strip() and collapsed and not collapsed[-1].strip():
                continue
            collapsed.append(l)
        kept = collapsed
        while kept and not kept[-1].strip():
            kept.pop()
        path.write_text("\n".join(kept + [""] + import_lines + [""] + rest).rstrip() + "\n",
                        encoding="utf-8")
        changed += 1
    return changed, skipped


def qualify_c_types() -> int:
    """`size_t` -> `std::size_t` in module units.

    The unqualified spellings live in the GLOBAL namespace and only exist in a
    translation unit that textually included <stddef.h> — which, in a module
    unit, means some vendor facade in the global module fragment happened to
    pull it in. `import std;` exports `std::size_t` and nothing else, so the
    unqualified form is a dependency on the CONTENT of a header nobody named.

    It held until `#include "vendor/asio.hpp"` became `import asio;`, at which
    point `utils/file/file.cppm` stopped compiling on `int64_t last_modified;` —
    a field that had never had anything to do with Asio.
    """
    changed = 0
    for path in list(SRC.rglob("*.cppm")) + list(SRC.rglob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        # MODULE_DECL_RE is anchored per LINE (it is used with .match on each
        # line elsewhere), so searching the whole text would only ever test
        # position 0 — and a module unit's first line is `module;`.
        if not any(MODULE_DECL_RE.match(l) for l in text.splitlines()):
            continue
        mask = _mask_comments_and_strings(text)
        out, last, hits = [], 0, 0
        for m in C_TYPE_RE.finditer(text):
            if mask[m.start()]:
                continue
            out.append(text[last:m.start()])
            out.append("std::" + m.group(1))
            last, hits = m.end(), hits + 1
        if hits:
            out.append(text[last:])
            path.write_text("".join(out), encoding="utf-8")
            changed += 1
    return changed


def normalize_module_units() -> int:
    """Move any `import` that landed in a global module fragment into the purview.

    A GMF may hold ONLY preprocessing directives, so an `import` between
    `module;` and the module declaration is ill-formed. It gets there naturally:
    `rewrite_consumers` replaces an `#include` line in place, and in an
    already-converted module unit that line was sitting in the GMF. Rewriting
    the include is right; leaving the import where the include was is not.
    """
    fixed = 0
    for path in list(SRC.rglob("*.cppm")) + list(SRC.rglob("*.cpp")):
        lines = path.read_text(encoding="utf-8").splitlines()
        if not lines or lines[0].strip() != "module;":
            continue
        decl = next((i for i, l in enumerate(lines) if MODULE_DECL_RE.match(l)), None)
        if decl is None:
            continue
        stray = [i for i, l in enumerate(lines[:decl]) if IMPORT_RE.match(l)]
        if not stray:
            continue
        moved = [lines[i] for i in stray]
        rest = [l for i, l in enumerate(lines) if i not in set(stray)]

        decl = next(i for i, l in enumerate(rest) if MODULE_DECL_RE.match(l))
        # Land them at the head of the existing import block, or right after the
        # declaration when there is none.
        at = next((i for i in range(decl + 1, len(rest)) if IMPORT_RE.match(rest[i])), None)
        if at is None:
            at = decl + 1
            rest[at:at] = [""] + moved
        else:
            rest[at:at] = moved

        while len(rest) > 1 and rest[0] == "" :
            rest.pop(0)
        # collapse a GMF that is now empty: `module;` followed by blanks
        out, blank = [], 0
        for l in rest:
            if l.strip() == "":
                blank += 1
                if blank > 1:
                    continue
            else:
                blank = 0
            out.append(l)
        path.write_text("\n".join(strip_empty_gmf(out)).rstrip() + "\n", encoding="utf-8")
        fixed += 1

    # A `module;` that introduces nothing can also come from a conversion whose
    # source had no vendor includes at all, so the sweep is over every unit
    # rather than only the ones touched above.
    for path in list(SRC.rglob("*.cppm")) + list(SRC.rglob("*.cpp")):
        lines = path.read_text(encoding="utf-8").splitlines()
        out = strip_empty_gmf(lines)
        if out != lines:
            path.write_text("\n".join(out).rstrip() + "\n", encoding="utf-8")
            fixed += 1
    return fixed


def strip_empty_gmf(lines: list[str]) -> list[str]:
    """Drop a `module;` that introduces nothing.

    An empty global module fragment is well-formed but says something false —
    it reads as "this unit reaches for the global module" when it no longer
    does. Once asio moved from a GMF `#include` to `import asio;`, several units
    were left with exactly that."""
    if not lines or lines[0].strip() != "module;":
        return lines
    i = 1
    while i < len(lines) and not lines[i].strip():
        i += 1
    if i < len(lines) and MODULE_DECL_RE.match(lines[i]):
        return lines[i:]
    return lines


def rename_module_prefix(old_top: str) -> int:
    """`module utils.x;` / `import utils.x;` -> the MODULE_PREFIX-carrying spelling.

    mcpp forbids a handful of top-level module names (core / util / common / std
    / detail / internal / base, modgraph/validate.cppm:48). `utils` is not among
    them, which is why the first converted batch built without a prefix — but a
    tree where some modules carry the project prefix and some do not is one
    where the rule is "whatever the last batch did". One rule, no exceptions.
    """
    pat = re.compile(rf'^((?:export )?(?:module|import)\s+){re.escape(old_top)}\.', re.M)
    touched = 0
    for path in list(SRC.rglob("*.cppm")) + list(SRC.rglob("*.cpp")):
        text = path.read_text(encoding="utf-8")
        new_text = pat.sub(rf'\g<1>{MODULE_PREFIX}{old_top}.', text)
        if new_text != text:
            path.write_text(new_text, encoding="utf-8")
            touched += 1
    return touched


def closure(seed_headers: list[str]) -> list[str]:
    """The headers that MUST convert together with `seed_headers`.

    A module unit may not `#include` in its purview and a HEADER may not
    `import` at all. So the moment a header's dependency becomes a module, that
    header has to become one too, and the requirement propagates up the include
    graph until it reaches a `.cpp` — which can `import` while staying a plain
    translation unit, and therefore ends the propagation.

    Seeds are src-relative header paths (`vendor/asio.hpp`, `core/rpc/types.hpp`).
    """
    files = [q for q in SRC.rglob("*") if q.suffix in {".hpp", ".cpp", ".cppm"}]
    by_rel = {str(q.relative_to(SRC)): q for q in files}

    includers: dict[Path, set[Path]] = {}
    for q in files:
        for line in q.read_text(encoding="utf-8", errors="replace").splitlines():
            m = INCLUDE_RE.match(line)
            if not m:
                continue
            tgt = by_rel.get(m.group(1))
            if tgt:
                includers.setdefault(tgt, set()).add(q)

    out: set[Path] = set()
    frontier = [by_rel[s] for s in seed_headers if s in by_rel]
    while frontier:
        cur = frontier.pop()
        for c in includers.get(cur, ()):
            if c.suffix == ".hpp" and c not in out:
                out.add(c)
                frontier.append(c)
    return sorted(str(q.relative_to(SRC)).removesuffix(".hpp") for q in out)


def main() -> int:
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 2
    if args[0] == "--normalize":
        print(f"normalized {normalize_module_units()} module units")
        print(f"qualified C types in {qualify_c_types()} module units")
        n, skipped = order_imports()
        print(f"ordered includes-before-imports in {n} plain TUs")
        for s in skipped:
            print(f"  SKIP {s} (preprocessor conditional in the prologue)")
        return 0
    if args[0] == "--rename-prefix":
        for top in args[1:]:
            print(f"{top}. -> {MODULE_PREFIX}{top}.: {rename_module_prefix(top)} files")
        return 0
    if args[0] == "--closure":
        for rel in closure(args[1:]):
            print(rel)
        return 0
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
    normalize_module_units()
    qualify_c_types()
    order_imports()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
