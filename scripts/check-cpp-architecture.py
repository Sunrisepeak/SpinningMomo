#!/usr/bin/env python3
"""Validate the module architecture without compiling the project.

The invariants are the ones the mcpp migration turns on:

  * every translation unit reaches the standard library through ONE door —
    `import std;` in a module unit, `#include "vendor/std.hpp"` in a plain one;
  * external `<>` includes appear only under src/vendor/, so the global module
    fragment has exactly one kind of entry;
  * no header units (`import <h>;`), which mcpp rejects outright and which the
    repository's abandoned first modularisation was built on;
  * a module interface exports declarations, not definitions: a non-template
    free function body belongs in the implementation unit.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
TESTS = ROOT / "tests"
CPP_SUFFIXES = {".hpp", ".cpp", ".cppm"}
MODULE_SUFFIXES = {".cppm"}

FORBIDDEN_TEXT = {
    r"\b(?:Core|Features|UI|Utils|Extensions|Vendor)::": "旧的大驼峰命名空间",
    r"\b(?:State|Types|UseCase)::": "已移除的职责命名空间",
    # `export module` / `import` are no longer forbidden — the mcpp migration
    # converts the tree to C++23 named modules bottom-up. What still must not
    # appear is a HEADER UNIT (`import <h>;` / `import "h";`): mcpp rejects
    # those outright (modgraph/scanner.cppm:702), and they are what the
    # previous, abandoned modularisation of this repo was built on.
    r"^\s*import\s*[<\"]": "C++ 头文件单元（mcpp 明确禁止）",
    r"\bnamespace\s*\{": "匿名命名空间",
    r"\b(?:web_view|d3_d|power_shell)\b": "非规范的复合命名空间拼写",
}

EXTERNAL_INCLUDE = re.compile(r"^\s*#include\s*<[^>]+>")

# `using X = ns::X;` — the same name on both sides.
#
# In the header world this is a redeclaration of one entity and does nothing. In
# a module it is a redefinition, and MSVC says so: `C1117: symbol 'TaskProgress'
# has already been defined`. Two of these sat in core/tasks/tasks.hpp and were
# what made that file untranslatable.
SELF_ALIAS = re.compile(r"^\s*using\s+(\w+)\s*=\s*[\w:]+::(\w+)\s*;", re.MULTILINE)


def blank_out_literals(text: str) -> str:
    """Replace comments and string literals with spaces, preserving offsets.

    Needed because several headers embed HLSL in raw string literals, and shader
    source looks exactly like C++ function definitions to a regex — three false
    positives on the "module interfaces hold declarations only" rule before this
    existed. Line and column numbers stay correct because every replaced
    character becomes a space (newlines survive)."""
    out = list(text)
    i, n = 0, len(text)

    def blank(a: int, b: int) -> None:
        for k in range(a, min(b, n)):
            if out[k] != "\n":
                out[k] = " "

    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif c == "R" and i + 1 < n and text[i + 1] == '"':
            # raw string: R"delim( … )delim"
            k = text.find("(", i + 2)
            if k < 0:
                i += 1
                continue
            delim = text[i + 2:k]
            close = ')' + delim + '"'
            j = text.find(close, k)
            j = n if j < 0 else j + len(close)
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
        blank(i, j)
        i = j
    return "".join(out)

MODULE_DECLARATION = re.compile(r"^(?:export\s+)?module\s+[A-Za-z_][\w.]*\s*;", re.MULTILINE)

NAMESPACE_DECLARATION = re.compile(
    r"^\s*namespace\s+([A-Za-z_][A-Za-z0-9_:]*)\s*\{", re.MULTILINE
)

TYPE_DECLARATION = re.compile(
    r"\b(?:struct|class|enum(?:\s+class)?)\s+([A-Za-z_][A-Za-z0-9_]*)"
)

LOWERCASE_TYPE_EXCEPTIONS = {
    "promise_type",
    "shared_state",
    "timeout_error",
    "ui_delay",
    "ui_task",
}

# A symbol whose provider must be named explicitly. Either spelling counts —
# the provider is a module now, but a plain .cpp still reaches it by import and
# an unconverted header still includes it.
REQUIRED_SYMBOL_PROVIDERS = {
    "utils::hash::": ('#include "utils/hash/xxhash.hpp"', "import sm.utils.hash.xxhash;"),
}

# `size_t` / `int64_t` and friends WITHOUT the std:: qualification, inside a
# module unit. `import std;` exports `std::size_t`; the unqualified spelling
# lives in the GLOBAL namespace and only exists in a translation unit that
# textually included <stddef.h> — which, in a module unit, means some vendor
# facade in the global module fragment happened to pull it in.
#
# That is a dependency on the CONTENT of a header nobody named. It held until
# `#include "vendor/asio.hpp"` became `import asio;`, at which point
# utils/file/file.cppm stopped compiling on `int64_t last_modified;` — a field
# that had never had anything to do with Asio.
UNQUALIFIED_C_TYPE = re.compile(
    r"(?<![\w:.>])("
    r"size_t|ptrdiff_t|intptr_t|uintptr_t"
    r"|u?int(?:8|16|32|64)_t"
    r")\b"
)

# A free function DEFINITION at namespace scope inside a module interface.
# Templates, `inline`, `constexpr`/`consteval` and class-member definitions are
# all legitimately part of an interface and are not matched: the target is the
# ordinary function whose body only makes the BMI bigger and every consumer's
# rebuild more likely.
IFACE_FUNCTION_BODY = re.compile(
    r"^(?!.*\b(?:inline|constexpr|consteval|template|friend|struct|class|enum|return)\b)"
    r"(?:auto|[A-Za-z_][\w:<>,\s\*&]*?)\s+"
    r"([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?"
    r"(?:->[^;{}]+?)?\{\s*$",
    re.MULTILINE,
)

CXX_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
    "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch",
    "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
    "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await",
    "co_return", "co_yield", "decltype", "default", "delete", "do", "double",
    "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float",
    "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace",
    "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
    "protected", "public", "reflexpr", "register", "reinterpret_cast", "requires",
    "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast",
    "struct", "switch", "synchronized", "template", "this", "thread_local", "throw",
    "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}


def cpp_files(directory: Path) -> list[Path]:
    if not directory.exists():
        return []
    return sorted(
        path
        for path in directory.rglob("*")
        if path.is_file() and path.suffix.lower() in CPP_SUFFIXES
    )


def report(errors: list[str], path: Path, line: int, message: str) -> None:
    errors.append(f"{path.relative_to(ROOT)}:{line}: {message}")


def validate_file(path: Path, errors: list[str]) -> None:
    text = path.read_text(encoding="utf-8-sig", errors="strict")
    # Comments and string literals blanked out, offsets preserved. Pattern rules
    # that look for CODE run against this; rules that look for directives or
    # raw text still use `text`.
    code = blank_out_literals(text)
    lines = text.splitlines()
    vendor_root = SRC / "vendor"
    is_vendor_facade = vendor_root in path.parents

    is_module_unit = path.suffix in MODULE_SUFFIXES or MODULE_DECLARATION.search(text)
    if not is_vendor_facade:
        # One door to the standard library, and which door depends on what the
        # unit is: a module unit may not `#include` in its purview, a plain
        # translation unit has no purview to import into.
        if is_module_unit:
            if "import std;" not in text:
                report(errors, path, 1, "模块单元缺少 import std;")
        elif '#include "vendor/std.hpp"' not in text:
            report(errors, path, 1, '缺少显式 #include "vendor/std.hpp"')
    if is_vendor_facade:
        facade_include = path.relative_to(SRC).as_posix()
        if f'#include "{facade_include}"' in text:
            report(errors, path, 1, "vendor 门面不能包含自身")

    for symbol, providers in REQUIRED_SYMBOL_PROVIDERS.items():
        if symbol in text and not any(p in text for p in providers):
            report(errors, path, 1, f"使用 {symbol} 时必须写明来源: {' 或 '.join(providers)}")

    if not is_module_unit and path.suffix == ".cpp":
        # In a PLAIN translation unit, every #include must come before every
        # import. Both orders are legal C++; only one compiles here.
        #
        # Importing a module whose global module fragment pulled in <windows.h>
        # and THEN including <windows.h> textually gives clang two parses of the
        # SDK, and the merge fails on winuser.h's unnamed structs
        # (`typedef struct {…} FLASHWINFO, *PFLASHWINFO;`):
        #
        #   winuser.h:4698: error: conflicting types for 'FlashWindowEx'
        #   winuser.h:4698: note: previous declaration is here   <- the same line
        #
        # The other direction merges fine. probes/p8-asio-windows is the
        # experiment: import-first fails, include-first passes, nothing else
        # changed.
        first_import = None
        for n, line in enumerate(lines, start=1):
            stripped = line.lstrip()
            if first_import is None and re.match(r"import\s", stripped):
                first_import = n
            elif first_import is not None and stripped.startswith("#include"):
                report(errors, path, n,
                       f"普通 TU 里 #include 必须全部在 import 之前"
                       f"（第 {first_import} 行已经 import）: {stripped}")
                break

    if is_module_unit:
        # A module name is a dot-separated sequence of IDENTIFIERS, so no
        # component may be a keyword. `core/http_server/static.cpp` maps to
        # `sm.core.http_server.static` under the path->name rule and clang
        # answers `expected a module name after 'module'`. The conversion script
        # suffixes such a component with `_`; this is the check that the rule
        # was applied.
        for match in MODULE_DECLARATION.finditer(text):
            name = match.group(0).split()[-1].rstrip(";")
            bad = [c for c in name.split(".") if c in CXX_KEYWORDS]
            if bad:
                line = text.count("\n", 0, match.start()) + 1
                report(errors, path, line,
                       f"模块名分量是 C++ 关键字: {name} (改成 {'/'.join(b + '_' for b in bad)})")

        for match in UNQUALIFIED_C_TYPE.finditer(text):
            line_text = text[text.rfind("\n", 0, match.start()) + 1 : text.find("\n", match.start())]
            if line_text.lstrip().startswith(("//", "*", "/*")):
                continue
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line,
                   f"模块单元里的 C 类型要写全: {match.group(1)} -> std::{match.group(1)}")

    # A self-alias is only a self-alias when the alias sits in the SAME namespace
    # as the entity it names. `using Direct3D11CaptureFrame =
    # winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame;` inside
    # `utils::graphics` imports a name from elsewhere and is perfectly fine — the
    # last segment matching is not enough to judge.
    for match in SELF_ALIAS.finditer(code):
        if match.group(1) != match.group(2):
            continue
        enclosing = ""
        for ns in NAMESPACE_DECLARATION.finditer(code):
            if ns.start() > match.start():
                break
            enclosing = ns.group(1)
        qualifier = match.group(0).split("=", 1)[1].strip().rstrip(";").rsplit("::", 1)[0]
        if enclosing and qualifier == enclosing:
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line,
                   f"自别名在模块里是重定义（C1117），删掉它: {match.group(0).strip()}")

    if path.suffix in MODULE_SUFFIXES:
        for match in IFACE_FUNCTION_BODY.finditer(code):
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line,
                   f"模块接口只放声明，实现移到同名 .cpp: {match.group(1)}(...)")

    for pattern, description in FORBIDDEN_TEXT.items():
        regex = re.compile(pattern, re.MULTILINE)
        for match in regex.finditer(text):
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line, f"仍包含{description}: {match.group(0).strip()}")

    for match in NAMESPACE_DECLARATION.finditer(text):
        namespace = match.group(1)
        bad_parts = [
            part
            for part in namespace.split("::")
            if part and (part != part.lower() or re.search(r"[A-Z]", part))
        ]
        if bad_parts:
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line, f"命名空间必须使用 lower_snake_case: {namespace}")
        keyword_parts = [part for part in namespace.split("::") if part in CXX_KEYWORDS]
        if keyword_parts:
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line, f"命名空间使用了 C++ 关键字: {namespace}")

    for match in TYPE_DECLARATION.finditer(text):
        type_name = match.group(1)
        if type_name[0].islower() and type_name not in LOWERCASE_TYPE_EXCEPTIONS:
            line = text.count("\n", 0, match.start()) + 1
            report(errors, path, line, f"项目类型必须使用 PascalCase: {type_name}")

    if not is_vendor_facade:
        for line_number, line in enumerate(lines, start=1):
            if EXTERNAL_INCLUDE.match(line):
                report(
                    errors,
                    path,
                    line_number,
                    f"外部头应通过精确的 vendor 门面引入: {line.strip()}",
                )


def main() -> int:
    # The findings are written in Chinese and this runs on a Windows runner,
    # where stdout defaults to the ANSI code page (cp1252) and `print` dies with
    # UnicodeEncodeError before showing a single one. A check that crashes
    # instead of reporting is worse than no check.
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(encoding="utf-8", errors="replace")
        except (AttributeError, ValueError):
            pass

    errors: list[str] = []

    module_interfaces = sorted(SRC.rglob("*.ixx")) + sorted(TESTS.rglob("*.ixx"))
    for path in module_interfaces:
        report(errors, path, 1, "仓库中仍存在 .ixx 模块接口")

    for path in cpp_files(SRC) + cpp_files(TESTS):
        validate_file(path, errors)

    xmake_text = (ROOT / "xmake.lua").read_text(encoding="utf-8-sig")
    for pattern, description in FORBIDDEN_TEXT.items():
        if re.search(pattern, xmake_text, re.MULTILINE):
            errors.append(f"xmake.lua: 仍包含{description}")

    if errors:
        print("C++ architecture check failed:")
        for error in errors:
            print(f"  {error}")
        return 1

    checked = len(cpp_files(SRC)) + len(cpp_files(TESTS))
    print(f"C++ architecture check passed ({checked} files).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
