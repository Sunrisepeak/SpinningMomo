# Architecture

> [!NOTE]
> This project recommends discussing requirements and direction through an issue before submitting a pull request.
>
> For new features, behavior changes, or larger refactors, please open an issue first and describe the problem being solved, the use case, and the expected outcome. This helps confirm whether the change fits the project direction before implementation begins.
>
> Pull requests are very welcome for issues with confirmed scope, clear bug fixes, documentation improvements, and technical challenges that have already been discussed. Unsolicited feature PRs may not be merged if they do not align with the project direction.

This project uses a hybrid architecture with a **C++23 native backend** and a **Vue 3
web frontend**. The backend is built from **C++23 named modules**: one `.cppm`
interface plus a matching `.cpp` implementation per unit, no precompiled header, and
no project header files at all outside `src/vendor/`.

External headers reach project code through exact facades under `src/vendor/`, in two
shapes divided by one thing — **macros**. Genuine third-party libraries (xxhash, webp,
sqlite, spdlog, rfl, uWebSockets, dkm) are `.cppm` wrapper modules, so their headers
are parsed once and only there. The Windows SDK, WIL and WebView2 stay headers, pulled
into each module's own global module fragment, because macros do not survive a BMI and
this tree uses about 600 of them (`FAILED`, `SUCCEEDED`, `IID_PPV_ARGS`, `WM_*`, …).

Dependencies come from the **official mcpp package index** (`mcpplibs/mcpp-index`);
the project no longer carries an index of its own.

For the full design philosophy, the C++ component breakdown, the module naming rules,
and the **fifteen machine-enforced architecture invariants**, check the root-level
**[`AGENTS.md`](https://github.com/ChanIok/SpinningMomo/blob/main/AGENTS.md)**.

## Prerequisites

The C++ backend has **two builds, both of record**: mcpp (primary, dependencies from
the official index) and xmake + vcpkg. Both reach the MSVC ABI **through LLVM** rather
than native `cl.exe`. That is forced by `import asio;`, not a preference: cl.exe cannot
round-trip asio's `io_context::service` — a nested class declared in-class and defined
out-of-class — through a BMI, and asio-as-a-module is precisely what stops its template
specialisations being re-instantiated in every importer. `probes/p6-asio-module` runs
under both toolchains and records the difference.

| Tool | Requirement | Notes |
|------|-------------|-------|
| **Visual Studio 2026 / Build Tools** | "Desktop development with C++" plus the C++ Clang tools | The IDE itself is optional |
| **Windows SDK** | 10.0.22621.0+ (Windows 11 SDK) | |
| **Git** | Latest | Clone vcpkg and fetch third-party dependencies |
| **mcpp** | 2026.8.17.1+ | Primary build system (`xlings install mcpp`; the version is pinned in `.xlings.json`) |
| **xmake** | 3.1.0+ | Second build system |
| **Node.js** | v20+ | Web frontend build and npm scripts |

### Install xmake

```powershell
# PowerShell (recommended)
irm https://xmake.io/psget.text | iex

# Or download from the official site
# https://xmake.io/#/getting_started?id=installation
```

### Set up vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git D:\dev\vcpkg  # path is up to you
cd D:\dev\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe integrate install
```

---

## Dependency Setup

### 1. Third-party dependencies

```bash
npm run fetch:third-party
```

### 2. npm dependencies

```bash
# Root (build script deps)
npm install

# Web frontend
npm ci --prefix web
```

### 3. Initialize xmake dependencies and apply patches

```bash
node scripts/patch-xmake-7554.js
node scripts/patch-xmake-clang-cl-cxx23.js
node scripts/patch-xmake-std-module-order.js

# Clang-cl + LLD (default)
xmake f --toolchain="clang-cl[llvm]" -y

# Or use MSVC
# xmake f --toolchain=msvc -y

xmake f -m release -y && xmake f -m debug -y
node scripts/patch-vcpkg.js
```

---

## Visual Studio Development (Optional)

To browse, edit, and debug the C++ code in Visual Studio, generate an
Xmake-managed solution:

```powershell
xmake vs
```

Then open:

```text
vsxmake2026\SpinningMomo.sln
```

---

## Build

> [!TIP]
> If you hit toolchain, dependency or environment trouble setting up locally, the two
> CI workflows —
> [`mcpp-windows.yml`](https://github.com/ChanIok/SpinningMomo/blob/main/.github/workflows/mcpp-windows.yml)
> and [`xmake-windows.yml`](https://github.com/ChanIok/SpinningMomo/blob/main/.github/workflows/xmake-windows.yml)
> — record the standard environment and build order that currently pass automated builds.

### Full Build (Recommended)

```bash
# One command: C++ Release + Web frontend + assemble dist/
npm run build
```

Output goes to `dist/`.

### Step-by-Step

```bash
# C++ backend — mcpp (primary)
mcpp build                 # debug
mcpp build --release

# C++ backend — xmake + vcpkg (second build)
xmake f -m debug --toolchain=clang-cl -y && xmake build
xmake release              # restores the debug config after a release build

# Web frontend
npm run build --prefix web

# Assemble dist/ (exe + web resources)
npm run build:prepare
```

### The seconds-long check to run before pushing

All four run on Linux, macOS and Windows, take under a second together, and every
error they catch saves a ~40-minute Windows CI round. The `preflight (linux)` job in
`mcpp-windows.yml` runs the first four; both `probe` and `full-build` `needs` it.

```bash
python3 scripts/check-module-graph.py        # the module graph is a DAG, and no import is missing
python3 scripts/check-cpp-architecture.py    # the architecture invariants
python3 scripts/check-asio-module-parity.py  # the xmake-side asio mirror matches the index descriptor
python3 scripts/check-build-parity.py        # both builds compile the same source under the same macros
bash    scripts/mcpp-preflight.sh            # the part of the probes Linux can decide
```

### Build Output Paths

| Type | Path |
|------|------|
| Debug | `build\windows\x64\debug\` |
| Release | `build\windows\x64\release\` |
| Packaged | `dist\` |

---

## Packaging

### Portable (ZIP)

```bash
npm run build:portable
```

### MSI Installer

Requires WiX Toolset v6:

```bash
dotnet tool install --global wix --version 6.0.2
wix extension add WixToolset.UI.wixext/6.0.2 --global
wix extension add WixToolset.BootstrapperApplications.wixext/6.0.2 --global
```

Then run:

```bash
npm run build:installer
```

---

## Web Frontend Development

Start the dev server (C++ backend needs to be running):

```bash
npm run dev:web
```

Vite dev server proxies `/rpc` and `/static` to the C++ backend (`localhost:51206`).

---

## Code Generation Scripts

Re-run these when their source files change:

| What changed | Run this script |
|--------------|-----------------|
| `src/migrations/*.sql` | `node scripts/generate-migrations.js` |
| `src/locales/*.json` | `node scripts/generate-embedded-locales.js` |
