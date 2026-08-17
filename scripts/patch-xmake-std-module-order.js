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
 */

const fs = require("node:fs");
const path = require("node:path");
const cp = require("node:child_process");

const MARKER = "-- PATCH(SpinningMomo): order every module BMI after std";

const BEFORE = [
  "    -- apply jobdeps",
  "    for jobname, deps in pairs(jobdeps) do",
].join("\n");

const AFTER = [
  `    ${MARKER}`,
  "    local _stdjobs = {}",
  "    for _, sourcefile in ipairs(_built_modules) do",
  "        local _m = mapper.get(target, sourcefile)",
  '        if _m.name == "std" or _m.name == "std.compat" then',
  "            table.insert(_stdjobs, _get_module_buildfilejob_for(target, sourcefile, moduletype))",
  "        end",
  "    end",
  "    for _, _stdjob in ipairs(_stdjobs) do",
  "        for _, buildfilejob in ipairs(buildfilejobs) do",
  "            if buildfilejob ~= _stdjob then",
  "                jobgraph:add_orders(_stdjob, buildfilejob)",
  "            end",
  "        end",
  "    end",
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

// The Lua rules sit next to xmake.exe on the Windows runner, but an
// xlings-managed install puts the executable behind a shim and keeps the rules
// in the share directory. Try both rather than assume the CI layout.
function findRulesFile() {
  const relative = path.join("rules", "c++", "modules", "builder.lua");
  const roots = [
    process.env.XMAKE_PROGRAM_DIR,
    path.dirname(getXmakeExe()),
    path.join(process.env.HOME || "", ".local", "share", "xmake"),
  ].filter(Boolean);
  for (const root of roots) {
    const candidate = path.join(root, relative);
    if (fs.existsSync(candidate)) return candidate;
  }
  fail(`target file not found under any of:\n  ${roots.join("\n  ")}`);
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
