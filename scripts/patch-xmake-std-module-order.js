/*
 * xmake patch: order every module BMI after the toolchain's `std` module.
 *
 * SYMPTOM
 *
 *   src\ui\floating_window\events.cppm(7,8): fatal error: module 'std' not found
 *   src\utils\crypto\crypto.cppm(3,8):       fatal error: module 'std' not found
 *   src\core\version.cppm(3,8):              fatal error: module 'std' not found
 *
 * A different file each run, always one whose ONLY module dependency is `std`.
 *
 * WHAT THE EVIDENCE SAYS
 *
 * The module map is right. Every unit that writes `import std;` gets
 * `-fmodule-file=std=...\std.pcm` on its command line, and the units that do
 * not (src/vendor/*.cppm — global module fragment only) do not. So xmake knows
 * the dependency exists.
 *
 * The BMI is right too. A diagnostic round left this on disk:
 *
 *   39768952  build/.gens/SpinningMomo/.../interfaces/eda4541c4cfe81c4/std.pcm
 *
 * 39 MB is a complete BMI — compiling MSVC's std.ixx with clang-cl succeeds.
 *
 * What is wrong is the JOB ORDER:
 *
 *   20:58:38.99  compiling.module.bmi.release std                  <- dispatched
 *   20:58:46.66  compiling.module.bmi.release sm.ui.floating_window.events
 *   20:58:47.24  events.cppm(7,8): fatal error: module 'std' not found
 *
 * 7.6 seconds is not long enough to compile the whole standard library, and the
 * four units dispatched at 46.6 are exactly the ones with nothing but `std` to
 * wait for. Units with project dependencies all waited correctly, so only the
 * edge to the toolchain-provided module is missing.
 *
 * Running the build twice does not fix it: `std`'s objectfile is never reached
 * before the build dies, so `should_build` sees a missing objectfile, decides
 * `std` is out of date, and rebuilds it — into the same race.
 *
 * THE PATCH
 *
 * Add the missing order edges explicitly, right before xmake applies the ones
 * it computed. `std` and `std.compat` are the only modules involved: every
 * other edge in this project's graph is already ordered correctly.
 *
 * xmake 3.1.0 (2026-08-08) is the current release and no later commit touches
 * this. Delete this script once one does.
 *
 * ONE TRAP, PAID FOR ONCE: patching a `rules/` tree that exists on disk is not
 * the same as patching the one xmake loads. This machine's xmake self-extracts
 * to /tmp/.xmakeNNNN/<version>/ and ignores the tree beside the executable
 * entirely — the patch applied, said so, and changed nothing. Hence
 * `os.programdir` below, and hence the `print`: an ordering patch that matches
 * nothing has to say so out loud.
 */

const fs = require("node:fs");
const path = require("node:path");
const cp = require("node:child_process");

const MARKER = "-- PATCH(SpinningMomo): order every module BMI after std";

const BEFORE = [
  "    -- apply jobdeps",
  "    for jobname, deps in pairs(jobdeps) do",
].join("\n");

// Feed the edges into `jobdeps` rather than calling `jobgraph:add_orders`
// directly, so they go through xmake's own application loop — one less thing
// that can differ from how the build system means it. The `print` is
// deliberate: an ordering patch that silently matches nothing is exactly the
// failure mode worth seeing in a log.
const AFTER = [
  `    ${MARKER}`,
  "    local _stdjobs = {}",
  "    for _, _sf in ipairs(_built_modules) do",
  "        local _m = mapper.get(target, _sf)",
  '        if _m and (_m.name == "std" or _m.name == "std.compat") then',
  "            table.insert(_stdjobs, _get_module_buildfilejob_for(target, _sf, moduletype))",
  "        end",
  "    end",
  "    local _ordered = 0",
  "    for _, _stdjob in ipairs(_stdjobs) do",
  "        for _, _bfj in ipairs(buildfilejobs) do",
  "            if _bfj ~= _stdjob then",
  "                jobdeps[_bfj] = jobdeps[_bfj] or {}",
  "                table.insert(jobdeps[_bfj], _stdjob)",
  "                _ordered = _ordered + 1",
  "            end",
  "        end",
  "    end",
  '    print("[std-order patch] %s: %d std job(s), %d module job(s), %d edges added",',
  "          target:fullname(), #_stdjobs, #buildfilejobs, _ordered)",
  "",
  "    -- apply jobdeps",
  "    for jobname, deps in pairs(jobdeps) do",
].join("\n");

function fail(message) {
  console.error(message);
  process.exit(1);
}

function getXmakeExe() {
  const command = process.platform === "win32" ? "where" : "which";
  const output = cp.execFileSync(command, ["xmake"], { encoding: "utf8" }).trim();
  const firstLine = output.split(/\r?\n/).find(Boolean);
  if (!firstLine) {
    fail("xmake executable not found.");
  }
  return firstLine;
}

// ASK XMAKE where its Lua lives. Guessing costs more than it looks: xmake may
// run from a directory next to the executable, or from a self-extracted copy
// under a temp path, and a `rules/` tree that exists on disk is not necessarily
// the one being loaded. Patching the wrong copy is silent — it applies, it says
// so, and it changes nothing. `os.programdir` is the authority.
function findRulesFile() {
  const relative = path.join("rules", "c++", "modules", "builder.lua");
  const roots = [];
  try {
    const out = cp.execFileSync(getXmakeExe(), ["l", "os.programdir"], { encoding: "utf8" });
    const dir = out.replace(/\x1b\[[0-9;]*m/g, "").trim().replace(/^"|"$/g, "");
    if (dir) roots.push(dir);
  } catch {
    /* fall through to the layout guesses below */
  }
  roots.push(process.env.XMAKE_PROGRAM_DIR, path.dirname(getXmakeExe()));
  for (const root of roots.filter(Boolean)) {
    const candidate = path.join(root, relative);
    if (fs.existsSync(candidate)) return candidate;
  }
  fail(`target file not found under any of:\n  ${roots.filter(Boolean).join("\n  ")}`);
}

function main() {
  const targetFile = findRulesFile();

  const original = fs.readFileSync(targetFile, "utf8");
  const normalized = original.replace(/\r\n/g, "\n");

  if (normalized.includes(MARKER)) {
    console.log(`already applied std module order patch: ${targetFile}`);
    return;
  }

  if (!normalized.includes(BEFORE)) {
    // Check the SUBSTANCE before giving up: the defect is a missing order edge
    // to the std module's BMI job. A builder that already names std while
    // building job orders has made that distinction some other way.
    if (/add_orders\([^)]*std/.test(normalized)) {
      console.log(
        `std module ordering already handled upstream: ${targetFile}\n` +
        "Nothing to patch. Delete this script once the pinned xmake floor is >= that release.",
      );
      return;
    }
    fail([
      `failed to match std module order patch rules: ${targetFile}`,
      "The installed xmake changed shape and does not obviously contain the fix either.",
      "Read build_modules_for_jobgraph() and decide: update BEFORE/AFTER, or drop this script.",
    ].join("\n"));
  }

  const eol = original.includes("\r\n") ? "\r\n" : "\n";
  fs.writeFileSync(targetFile, normalized.replace(BEFORE, AFTER).replace(/\n/g, eol), "utf8");
  console.log(`patched std module build order: ${targetFile}`);
}

main();
