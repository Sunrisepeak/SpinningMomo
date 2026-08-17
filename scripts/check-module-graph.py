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

    check-module-graph.py            # verify (exit 1 on a cycle)
    check-module-graph.py --graph    # print the interface DAG, deepest first
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"

EXPORT_MODULE = re.compile(r"^export\s+module\s+([\w.]+)\s*;", re.MULTILINE)
IMPL_MODULE = re.compile(r"^module\s+([\w.]+)\s*;", re.MULTILINE)
IMPORT = re.compile(r"^(?:export\s+)?import\s+([\w.:]+)\s*;", re.MULTILINE)

# Provided by the toolchain or by a package, not by this tree.
EXTERNAL = {"std", "std.compat", "asio"}


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
