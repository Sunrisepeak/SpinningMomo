#!/usr/bin/env python3
"""The two builds must compile the same source under the same contract.

SpinningMomo builds under mcpp and under xmake, and the point of keeping both is
that anything passing both is a property of the SOURCE. That only holds while the
two agree about what they are compiling and with which macros — and nothing in
either build system notices when they stop agreeing. A define that lands in one
manifest and not the other does not fail: it silently compiles a different
program, and the difference surfaces as a struct layout mismatch or an inline
definition that should have been extern.

Two invariants, both cheap:

  1. THE DEFINE SETS ARE EQUAL. Several of them are interface contracts rather
     than private switches — `ASIO_SEPARATE_COMPILATION` decides whether a
     consumer re-emits what the package compiled out-of-line, `LIBUS_USE_LIBUV`
     changes the layout of `us_loop_t`, `SPDLOG_COMPILED_LIB` selects the
     extern-template path. Disagreeing on any of them is a runtime bug, not a
     build error.

  2. BOTH COMPILE THE WHOLE TREE. mcpp globs `src/**/*.{cpp,cppm}`; xmake must
     have the matching `add_files` lines, plus the asio module unit it has to
     build itself because vcpkg ships headers only.

Deliberately NOT checked: the flag dialect. mcpp drives `clang++` (GNU spelling)
and xmake drives `clang-cl` (MSVC spelling), so `-Wl,/subsystem:windows` and
`/SUBSYSTEM:WINDOWS` are the same instruction written twice. Comparing those
would be comparing spellings, not contracts.

    check-build-parity.py        # exit 1 on a mismatch
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MCPP = ROOT / "mcpp.toml"
XMAKE = ROOT / "xmake.lua"


def mcpp_defines() -> set[str]:
    text = MCPP.read_text(encoding="utf-8")
    out: set[str] = set()
    for block in re.finditer(r"^defines\s*=\s*\[(.*?)^\]", text, re.S | re.M):
        body = re.sub(r"#[^\n]*", "", block.group(1))     # strip comments
        out |= set(re.findall(r'"([^"]+)"', body))
    return out


def xmake_defines() -> set[str]:
    text = XMAKE.read_text(encoding="utf-8")
    # Only the SpinningMomo target; tests/ has its own smaller set on purpose.
    target = text.split('target("SpinningMomo")', 1)[-1]
    target = target.split('target("', 1)[0]
    out: set[str] = set()
    for call in re.finditer(r"add_defines\((.*?)\)", target, re.S):
        body = re.sub(r"--[^\n]*", "", call.group(1))
        out |= set(re.findall(r'"([^"]+)"', body))
    return out


def main() -> int:
    problems: list[str] = []

    m, x = mcpp_defines(), xmake_defines()
    if not m or not x:
        problems.append(f"could not read the define sets (mcpp={len(m)}, xmake={len(x)}) "
                        f"— the manifest shape changed and this check needs updating")
    for only, where, other in ((m - x, "mcpp.toml", "xmake.lua"),
                               (x - m, "xmake.lua", "mcpp.toml")):
        for d in sorted(only):
            problems.append(f"define `{d}` is in {where} but not {other}")

    xmake_text = XMAKE.read_text(encoding="utf-8")
    for glob in ('add_files("src/**.cpp")', 'add_files("src/**.cppm")',
                 'add_files("third_party/asio-module/asio.cppm")',
                 'add_files("third_party/asio-module/asio_src.cpp")'):
        if glob not in xmake_text:
            problems.append(f"xmake.lua is missing {glob} — the two builds would not "
                            f"compile the same set of units")

    if 'set_policy("build.c++.modules", true)' not in xmake_text:
        problems.append("xmake.lua does not enable build.c++.modules")

    if problems:
        print("build parity FAILED:")
        for p in problems:
            print(f"  {p}")
        return 1

    print(f"build parity OK — {len(m)} defines shared by both manifests, "
          f"both compile src/**.{{cpp,cppm}}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
