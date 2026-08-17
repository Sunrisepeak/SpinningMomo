/*
 * Temporary xmake patch for clang-cl C++23 language selection.
 *
 * xmake v3.0.9 checks the MSVC-only `-std:c++23` spelling for clang-cl.
 * clang-cl rejects it, so xmake falls back to `-std:c++latest`, which selects
 * C++26 in Clang 21. Pass the standard directly to the Clang frontend instead.
 */

const fs = require("node:fs");
const path = require("node:path");
const cp = require("node:child_process");

const BEFORE = [
  '        if self:name() == "clang_cl" then',
  '            cxx23 = {"-std:c++23", "-std:c++latest"}',
  "        end",
].join("\n");

const AFTER = [
  '        if self:name() == "clang_cl" then',
  '            cxx23 = {"-Xclang -std=c++23", "-std:c++latest"}',
  "        end",
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

function detectEol(text) {
  return text.includes("\r\n") ? "\r\n" : "\n";
}

function main() {
  const xmakeExe = getXmakeExe();
  const xmakeRoot = path.dirname(xmakeExe);
  const targetFile = path.join(
    xmakeRoot,
    "modules",
    "core",
    "tools",
    "cl.lua",
  );

  if (!fs.existsSync(targetFile)) {
    fail(`target file not found: ${targetFile}`);
  }

  const original = fs.readFileSync(targetFile, "utf8");
  const normalized = original.replace(/\r\n/g, "\n");

  if (normalized.includes(AFTER) && !normalized.includes(BEFORE)) {
    console.log(`already applied clang-cl C++23 patch: ${targetFile}`);
    return;
  }

  if (!normalized.includes(BEFORE)) {
    // The upstream fix landed in some shape other than this patch's AFTER text.
    // Check the SUBSTANCE: the bug was that clang-cl was offered the MSVC-only
    // `-std:c++23`, which it rejects, so xmake fell back to `-std:c++latest`
    // (C++26 on Clang 21+). A file that hands clang-cl `-std=c++23` through the
    // Clang frontend at all has already made that distinction.
    if (normalized.includes("clang_cl") && normalized.includes("-std=c++23")) {
      console.log(
        `clang-cl C++23 selection already handled upstream: ${targetFile}\n` +
        "Nothing to patch. Delete this script once the pinned xmake floor is >= that release.",
      );
      return;
    }
    fail([
      `failed to match clang-cl C++23 patch rules: ${targetFile}`,
      "The installed xmake changed shape and does not obviously contain the fix either.",
      "Read the file and decide: update BEFORE/AFTER, or drop this script.",
    ].join("\n"));
  }

  const patched = normalized.replace(BEFORE, AFTER);
  const eol = detectEol(original);
  fs.writeFileSync(targetFile, patched.replace(/\n/g, eol), "utf8");
  console.log(`patched clang-cl C++23 selection: ${targetFile}`);
}

main();
