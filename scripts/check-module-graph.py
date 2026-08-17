#!/usr/bin/env python3
"""Reject a cyclic module graph before a Windows CI round pays for it.

mcpp validates this itself, but only inside a build — which on this project
means a 40-minute compile on a Windows runner. The same fact is derivable from
the source text in under a second on any machine, so it belongs here too.

THE MODEL IS mcpp's, deliberately (src/modgraph/scanner.cppm resolve_graph +
graph.cppm topo_sort):

  * a NODE is a translation unit, not a module;
  * `export module X;` makes that unit the PRODUCER of X;
  * `module X;` (an implementation unit) produces nothing;
  * every `import Y;` is an edge from THIS unit to Y's producer unit.

The per-unit granularity matters, and getting it wrong sends you chasing
phantoms. Two implementation units that import each other's modules —
`preview/rendering.cpp` imports `sm.features.preview.viewport` while
`preview/viewport.cpp` imports `sm.features.preview.rendering` — look circular
if you collapse each module to a single node. They are not: the edges run
rendering.cpp -> viewport.cppm and viewport.cpp -> rendering.cppm, and the two
INTERFACES depend on neither. Ten such pairs exist in this tree and all ten are
fine.

What is not fine is a cycle among interfaces, and this project had exactly one.
Modularising AppState turned its 29 forward declarations into imports, which
closed:

    app_state -> ui.notification_window.state -> ui.notification_window.types
              -> core.notifications.types -> app_state

The header world had hidden it behind include guards. See the note in
src/core/notifications/types.cppm for how it was cut.

It also checks the other half of the same fact: that every project namespace a
unit NAMES is provided by a module that unit can actually see. A module import
is not transitive — `import A;` where A does `import B;` does NOT make B's
exports visible, only `export import B;` does. In the header world a transitive
`#include` did that job silently, so the dependency was never written down:

    features/gallery/watcher/watcher.cpp names features::gallery::recovery::
    StartupRecoveryPlan while importing only …recovery.service

which compiles for exactly as long as something else happens to pull the types
module in. Each instance of this costs a 40-minute Windows round to discover, so
it is worth a second of Linux.

The UNQUALIFIED half of that is the same fact wearing a disguise, and it is the
one that actually reached CI:

    src/features/overlay/capture.cpp:61: error: declaration of
    'WM_APPLY_CAPTURE_SIZE' must be imported from module
    'sm.features.overlay.types' before it is required

capture.cpp sits inside `namespace features::overlay::capture`, so unqualified
lookup walks out to `features::overlay` and finds the constant — REACHABLE,
because something in the import graph pulls the types module in transitively,
but not VISIBLE, because nothing capture.cpp imports exports it. There is no
`features::overlay::` on the use site for a qualified-name scan to catch, so
this needs the entity names themselves: what each interface exports, and whether
the namespace a unit has open would let unqualified lookup wander into it.

Eleven uses across nine implementation units had it, all the same shape — their
own `.cppm` `import`s the types module instead of `export import`ing it, so the
interface sees the names and the implementation does not.

Resolving against the ENTITY rather than the namespace is what makes the
qualified half work too, and the difference is not academic:

    src/features/recording/encoder_loop.cpp:343: error: declaration of
    'EncoderContext' must be imported from module 'sm.utils.media.state'
    before it is required

encoder_loop.cpp writes `utils::media::encoder::EncoderContext` in full and
imports `sm.utils.media.encoder`, which opens that namespace and names the type
in its own signatures — but does not define it. Six modules open
`utils::media::`; asking only "is the namespace reachable" says yes and misses
this every time.

    check-module-graph.py            # verify (exit 1 on a cycle or a missing import)
    check-module-graph.py --graph    # print the interface DAG, deepest first
"""

from __future__ import annotations

import argparse
import importlib.util
import re
import sys
from collections import defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"


def _load_sibling(stem: str):
    """The sibling guard owns the literal blanker; one definition, two users."""
    path = Path(__file__).with_name(f"{stem}.py")
    spec = importlib.util.spec_from_file_location(stem.replace("-", "_"), path)
    module = importlib.util.module_from_spec(spec)          # type: ignore[arg-type]
    spec.loader.exec_module(module)                          # type: ignore[union-attr]
    return module


blank_out_literals = _load_sibling("check-cpp-architecture").blank_out_literals

EXPORT_MODULE = re.compile(r"^export\s+module\s+([\w.]+)\s*;", re.MULTILINE)
IMPL_MODULE = re.compile(r"^module\s+([\w.]+)\s*;", re.MULTILINE)
IMPORT = re.compile(r"^(?:export\s+)?import\s+([\w.:]+)\s*;", re.MULTILINE)

# Provided by the toolchain or by a package, not by this tree.
EXTERNAL = {"std", "std.compat", "asio"}

EXPORT_MODULE_DECL = re.compile(r"^export\s+module\s+([\w.]+)\s*;", re.MULTILINE)
REEXPORT = re.compile(r"^export\s+import\s+([\w.]+)\s*;", re.MULTILINE)
PLAIN_IMPORT = re.compile(r"^import\s+([\w.]+)\s*;", re.MULTILINE)
EXPORT_NAMESPACE = re.compile(r"^export namespace ([\w:]+)", re.MULTILINE)
NAMESPACE_DECL = re.compile(r"^\s*(?:export\s+)?namespace\s+([\w:]+)", re.MULTILINE)
# A qualified use of one of the project's own top-level namespaces.
PROJECT_USE = re.compile(
    r"(?<![\w:])((?:core|features|ui|utils|extensions)(?:::\w+)*)::\w")


class Unit:
    __slots__ = ("path", "provides", "implements", "requires")

    def __init__(self, path: Path, provides: str | None,
                 implements: str | None, requires: set[str]):
        self.path = path
        self.provides = provides
        self.implements = implements
        self.requires = requires

    @property
    def label(self) -> str:
        return str(self.path.relative_to(ROOT))


def scan() -> list[Unit]:
    units: list[Unit] = []
    for path in sorted(list(SRC.rglob("*.cppm")) + list(SRC.rglob("*.cpp"))):
        text = path.read_text(encoding="utf-8", errors="replace")
        exp = EXPORT_MODULE.search(text)
        impl = None if exp else IMPL_MODULE.search(text)
        requires = {m for m in IMPORT.findall(text) if m not in EXTERNAL}
        units.append(Unit(path, exp.group(1) if exp else None,
                          impl.group(1) if impl else None, requires))
    return units


def build(units: list[Unit]):
    producer: dict[str, int] = {}
    dupes: list[str] = []
    for i, u in enumerate(units):
        if u.provides:
            if u.provides in producer:
                dupes.append(f"module '{u.provides}' provided by both "
                             f"{units[producer[u.provides]].label} and {u.label}")
            else:
                producer[u.provides] = i

    edges: list[tuple[int, int]] = []   # (consumer, producer)
    unknown: list[str] = []
    for i, u in enumerate(units):
        # An implementation unit implicitly imports its own interface.
        if u.implements:
            if u.implements in producer:
                edges.append((i, producer[u.implements]))
            else:
                unknown.append(f"{u.label}: implements '{u.implements}', "
                               f"which no unit provides")
        for req in sorted(u.requires):
            if req in producer:
                edges.append((i, producer[req]))
            elif not req.startswith(":"):     # a partition is intra-module
                unknown.append(f"{u.label}: imports '{req}', which no unit provides")
    return producer, edges, dupes, unknown


def topo(n: int, edges: list[tuple[int, int]]) -> list[int]:
    """Kahn, exactly as mcpp does it. Returns the units left in a cycle."""
    indeg = [0] * n
    adj: dict[int, list[int]] = defaultdict(list)
    for consumer, prod in edges:
        indeg[consumer] += 1
        adj[prod].append(consumer)
    queue = deque(i for i in range(n) if indeg[i] == 0)
    seen = 0
    while queue:
        u = queue.popleft()
        seen += 1
        for v in adj[u]:
            indeg[v] -= 1
            if indeg[v] == 0:
                queue.append(v)
    return [i for i in range(n) if indeg[i] > 0] if seen != n else []


def interface_cycles(units: list[Unit], producer: dict[str, int]) -> list[list[str]]:
    """Cycles among INTERFACES only — the ones that are real design problems."""
    graph = {u.provides: {r for r in u.requires if r in producer}
             for u in units if u.provides}
    colour: dict[str, int] = {}
    stack: list[str] = []
    found: list[list[str]] = []

    def walk(node: str) -> None:
        colour[node] = 1
        stack.append(node)
        for nxt in sorted(graph.get(node, ())):
            if colour.get(nxt, 0) == 0:
                walk(nxt)
            elif colour.get(nxt) == 1:
                found.append(stack[stack.index(nxt):] + [nxt])
        stack.pop()
        colour[node] = 2

    sys.setrecursionlimit(10000)
    for m in sorted(graph):
        if colour.get(m, 0) == 0:
            walk(m)
    return found


def missing_imports(units: list[Unit]) -> list[str]:
    """A unit names a project namespace no module it can see exports."""
    provider_of: dict[str, set[str]] = defaultdict(set)
    reexports: dict[str, set[str]] = defaultdict(set)
    for u in units:
        text = u.path.read_text(encoding="utf-8", errors="replace")
        m = EXPORT_MODULE_DECL.search(text)
        if not m:
            continue
        for ns in EXPORT_NAMESPACE.findall(text):
            provider_of[ns].add(m.group(1))
        reexports[m.group(1)] |= set(REEXPORT.findall(text))

    def visible(seeds: set[str]) -> set[str]:
        seen: set[str] = set()
        stack = list(seeds)
        while stack:
            mod = stack.pop()
            if mod in seen:
                continue
            seen.add(mod)
            stack.extend(reexports.get(mod, ()))
        return seen

    out: list[str] = []
    for u in units:
        text = u.path.read_text(encoding="utf-8", errors="replace")
        lines = text.splitlines()
        own = {u.provides, u.implements} - {None}
        reach = visible(set(PLAIN_IMPORT.findall(text))
                        | set(REEXPORT.findall(text)) | own)          # type: ignore[arg-type]
        own_ns = set(NAMESPACE_DECL.findall(text))
        reported: set[str] = set()
        for m in PROJECT_USE.finditer(text):
            ns = m.group(1)
            if ns in reported:
                continue
            line_no = text.count("\n", 0, m.start())
            line = lines[line_no] if line_no < len(lines) else ""
            if re.match(r"^\s*(export\s+)?namespace\b", line):
                continue                                   # a declaration, not a use
            if line.lstrip().startswith(("//", "*", "/*")):
                continue                                   # a comment
            if any(ns == o or o.startswith(ns + "::") or ns.startswith(o + "::")
                   for o in own_ns):
                continue                                   # our own subtree
            providers = provider_of.get(ns)
            if providers and not (providers & reach):
                reported.add(ns)
                out.append(f"{u.label}:{line_no + 1}: names `{ns}::` but imports no module "
                           f"that exports it (try: {sorted(providers)[0]})")
    return out


# ── the unqualified half ─────────────────────────────────────────────────────

NS_BLOCK = re.compile(r"^export namespace ([\w:]+)\s*\{", re.MULTILINE)
NS_OPEN = re.compile(r"^\s*(?:export\s+)?namespace\s+([\w:]+)\s*\{", re.MULTILINE)

_TAG = re.compile(r"\b(?:struct|class|union|enum(?:\s+class)?)\s+(\w+)\s*$")
_ALIAS = re.compile(r"\busing\s+(\w+)\s*=")
_CONCEPT = re.compile(r"\bconcept\s+(\w+)\s*=")
_VAR = re.compile(r"^\s*(?:export\s+)?"
                  r"(?:(?:inline|constexpr|const|static|extern)\s+)+"
                  r"[\w:]+(?:\s*<[^;]*>)?\s*[*&]?\s*(\w+)\s*(?:=|\[)")
# `auto name(...)` — the project writes every function in trailing-return form,
# so this one spelling covers declarations and definitions alike. Tried last, so
# `constexpr auto kFoo = …` is a variable and `using Fn = auto (*)(int) -> void`
# is an alias.
_FUNC = re.compile(r"\bauto\s+(\w+)\s*\(")

# Preceded by one of these, an identifier is a DECLARATOR (a member, a
# parameter, a loop variable), not a use — `bool is_initialized = false;` must
# not read as a use of some other namespace's `is_initialized()`. Type
# specifiers belong here; `const`/`static`/`return` and friends do not, because
# a real use follows them (`const Resolution&`, `return WindowInfo{}`).
_BUILTIN_TYPE = {"bool", "char", "char8_t", "char16_t", "char32_t", "wchar_t",
                 "short", "int", "long", "float", "double", "void", "auto",
                 "signed", "unsigned"}
_NOT_A_TYPE = {"const", "constexpr", "consteval", "constinit", "static", "inline",
               "extern", "mutable", "volatile", "return", "case", "new", "delete",
               "throw", "co_await", "co_return", "co_yield", "sizeof", "using",
               "typename", "struct", "class", "enum", "union", "export", "import",
               "namespace", "else", "do", "try", "catch", "noexcept", "explicit",
               "friend", "virtual", "typedef", "template", "and", "or", "not",
               "if", "while", "for", "switch", "requires", "static_cast",
               "reinterpret_cast", "const_cast", "dynamic_cast"}


def _brace_body(text: str, open_brace: int) -> str:
    depth, i = 0, open_brace
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[open_brace + 1:i]
        i += 1
    return text[open_brace + 1:]


def _top_level_statements(body: str):
    """Statements at brace depth 0 of a namespace body — nothing from inside a
    struct, a function, or a parameter list."""
    depth = start = i = 0
    n = len(body)
    while i < n:
        c = body[i]
        if c == "{" and depth == 0:
            yield body[start:i]                     # the declaration head
            d, i = 1, i + 1
            while i < n and d:
                d += (body[i] == "{") - (body[i] == "}")
                i += 1
            start = i
            continue
        if c in "{([":
            depth += 1
        elif c in "})]":
            depth -= 1
        elif c == ";" and depth == 0:
            yield body[start:i]
            start = i + 1
        i += 1


def _entities(body: str) -> set[str]:
    out: set[str] = set()
    for stmt in _top_level_statements(body):
        s = stmt.strip()
        if not s:
            continue
        for rx in (_TAG, _ALIAS, _CONCEPT, _VAR, _FUNC):
            m = rx.search(s)
            if m:
                out.add(m.group(1))
                break
    return out


def _namespace_entities(text: str, pattern: re.Pattern[str]) -> dict[str, set[str]]:
    out: dict[str, set[str]] = defaultdict(set)
    for m in pattern.finditer(text):
        out[m.group(1)] |= _entities(_brace_body(text, m.end() - 1))
    return out


def entity_missing_imports(units: list[Unit]) -> list[str]:
    """A unit names an ENTITY that only a module it cannot see exports.

    Reachable-but-not-visible: the name resolves during the build only because
    someone else's import chain drags the module in, and it stops resolving the
    day that chain changes.

    This is finer than missing_imports() above, which asks only whether the
    NAMESPACE is reachable. That is not enough when several modules open the same
    namespace, which is the normal shape here:

        features/recording/encoder_loop.cpp names utils::media::encoder::
        EncoderContext while importing sm.utils.media.encoder — which opens that
        namespace, and mentions the type in its signatures, but does not define
        it. The definition is in sm.utils.media.state.

    So both halves — qualified and unqualified — resolve against the entity.
    """
    provider: dict[str, set[tuple[str, str]]] = defaultdict(set)   # name -> {(ns, module)}
    reexports: dict[str, set[str]] = defaultdict(set)
    texts: dict[Path, str] = {}

    for u in units:
        text = texts[u.path] = blank_out_literals(
            u.path.read_text(encoding="utf-8", errors="replace"))
        if not u.provides:
            continue
        reexports[u.provides] |= set(REEXPORT.findall(text))
        for ns, names in _namespace_entities(text, NS_BLOCK).items():
            for name in names:
                provider[name].add((ns, u.provides))
        # Two entities live at global scope with a per-declaration `export`
        # rather than inside an `export namespace` block — `Application` and
        # `Logger`. Nothing encloses them, so `""` stands for the global
        # namespace and every unit has it open.
        for m in re.finditer(r"^export\s+(?:class|struct|enum(?:\s+class)?|union)\s+(\w+)",
                             text, re.MULTILINE):
            provider[m.group(1)].add(("", u.provides))

    def visible(seeds: set[str]) -> set[str]:
        seen: set[str] = set()
        stack = list(seeds)
        while stack:
            mod = stack.pop()
            if mod in seen:
                continue
            seen.add(mod)
            stack.extend(reexports.get(mod, ()))
        return seen

    out: list[str] = []
    for u in units:
        text = texts[u.path]
        opened = set(NS_OPEN.findall(text))
        if not opened:
            continue
        own = {u.provides, u.implements} - {None}
        reach = visible(set(PLAIN_IMPORT.findall(text))
                        | set(REEXPORT.findall(text)) | own)        # type: ignore[arg-type]
        # Names this unit declares itself: lookup stops here, not at the import.
        mine: set[str] = set()
        for names in _namespace_entities(text, NS_OPEN).values():
            mine |= names
        # …and names bound LOCALLY anywhere in the unit — a parameter
        # `SQLite::Statement& query`, a loop variable `for (int stop : …)`. An
        # unqualified use of one of those is not a namespace lookup at all, and
        # some of them collide with a real export elsewhere in the tree. There is
        # no scope analysis here, so a name used as a declarator even once is
        # dropped from the unqualified half. The qualified half is unaffected and
        # stays exact.
        mine |= {mm.group(1) for mm in
                 re.finditer(r"(?:[\w>&*\]]|\b(?:" + "|".join(sorted(_BUILTIN_TYPE))
                             + r"))\s+([A-Za-z_]\w*)\b\s*(?:[=;,)\[:]|\{)", text)
                 if mm.group(1) not in _NOT_A_TYPE}
        lines = text.splitlines()
        reported: set[tuple[str, str]] = set()
        for m in re.finditer(r"(?<![\w:.])(?<!->)((?:\w+::)*)([A-Za-z_]\w*)\b(?!\s*::)", text):
            qualifier, name = m.group(1)[:-2], m.group(2)
            if (qualifier, name) in reported or name not in provider:
                continue
            if not qualifier and name in mine:
                continue                # this unit declares it; lookup stops here
            if not qualifier:
                before = text[:m.start()].rstrip()
                tail = re.search(r"[\w>&*]+$", before)
                if tail:
                    word = re.search(r"\w+$", tail.group(0))
                    if not word or word.group(0) in _BUILTIN_TYPE:
                        continue        # `std::atomic<bool> x`, `T& p`, `int i`
                    if word.group(0) not in _NOT_A_TYPE:
                        continue        # `SomeType name` — a declarator
                # Unqualified lookup only escapes into an ENCLOSING namespace —
                # and the global one (ns == "") encloses everything.
                cands = [(ns, mod) for ns, mod in provider[name]
                         if not ns or any(o == ns or o.startswith(ns + "::")
                                          for o in opened)]
                how = "unqualified"
            else:
                # Written in full, or relative to a namespace this unit has open.
                cands = [(ns, mod) for ns, mod in provider[name]
                         if ns == qualifier
                         or (ns.endswith("::" + qualifier)
                             and any(ns.startswith(o + "::") or ns == o for o in opened))]
                how = f"as `{qualifier}::`"
            if not cands or any(mod in reach for _, mod in cands):
                continue
            reported.add((qualifier, name))
            line_no = text.count("\n", 0, m.start())
            out.append(f"{u.label}:{line_no + 1}: names `{name}` {how}, but nothing "
                       f"it imports exports it (try: {cands[0][1]})")
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--graph", action="store_true", help="print the interface DAG")
    args = ap.parse_args()

    units = scan()
    producer, edges, dupes, unknown = build(units)

    problems = list(dupes)
    problems += unknown

    for cyc in interface_cycles(units, producer):
        problems.append("interface cycle: " + " -> ".join(cyc))

    problems += missing_imports(units)
    problems += entity_missing_imports(units)

    stuck = topo(len(units), edges)
    if stuck:
        problems.append(
            f"the unit graph does not topologically sort — {len(stuck)} units remain, "
            f"starting with: " + ", ".join(units[i].label for i in stuck[:5]))

    modules = sum(1 for u in units if u.provides)
    impls = sum(1 for u in units if u.implements)
    plain = len(units) - modules - impls

    if problems:
        print("module graph check FAILED:")
        for p in problems:
            print(f"  {p}")
        return 1

    print(f"module graph OK — {modules} interfaces, {impls} implementation units, "
          f"{plain} plain TUs, {len(edges)} edges, acyclic.")

    if args.graph:
        depth: dict[str, int] = {}
        graph = {u.provides: {r for r in u.requires if r in producer}
                 for u in units if u.provides}

        def d(m: str, seen: frozenset[str] = frozenset()) -> int:
            if m in depth:
                return depth[m]
            if m in seen:
                return 0
            depth[m] = 1 + max((d(x, seen | {m}) for x in graph.get(m, ())), default=-1)
            return depth[m]

        for m in graph:
            d(m)
        for m, k in sorted(depth.items(), key=lambda kv: (-kv[1], kv[0]))[:25]:
            print(f"  depth {k:3}  {m}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
