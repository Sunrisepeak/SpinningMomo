/*
 * Where mcpp puts a built binary.
 *
 * mcpp writes to `target/<triple>/<fingerprint>/bin/<name>` — the fingerprint
 * covers everything that would change the output (profile, toolchain, feature
 * set), so one triple directory can hold several. There is no stable symlink
 * and no `--print-path`, so the path has to be discovered rather than spelled.
 *
 * Newest wins: a repository that has built both debug and release holds two,
 * and the one the caller means is the one just produced.
 */

const fs = require("fs");
const path = require("path");

const PROJECT_DIR = path.join(__dirname, "..");

/** Every `target/<triple>/<fp>/bin/<name>` that exists, newest first. */
function findArtifacts(name, projectDir = PROJECT_DIR) {
  const targetDir = path.join(projectDir, "target");
  if (!fs.existsSync(targetDir)) return [];

  const found = [];
  for (const triple of fs.readdirSync(targetDir, { withFileTypes: true })) {
    if (!triple.isDirectory()) continue;
    const tripleDir = path.join(targetDir, triple.name);
    for (const fingerprint of fs.readdirSync(tripleDir, { withFileTypes: true })) {
      if (!fingerprint.isDirectory()) continue;
      const candidate = path.join(tripleDir, fingerprint.name, "bin", name);
      if (fs.existsSync(candidate)) {
        found.push({ path: candidate, mtime: fs.statSync(candidate).mtimeMs });
      }
    }
  }
  return found.sort((a, b) => b.mtime - a.mtime).map((entry) => entry.path);
}

/** The newest one, or null. Callers decide what a miss means. */
function findArtifact(name, projectDir = PROJECT_DIR) {
  return findArtifacts(name, projectDir)[0] ?? null;
}

module.exports = { findArtifact, findArtifacts, PROJECT_DIR };
