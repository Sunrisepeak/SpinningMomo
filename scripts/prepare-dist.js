const path = require("path");
const fs = require("fs");
const { findArtifact } = require("./mcpp-artifact");

function main() {
  const projectDir = path.join(__dirname, "..");
  const distDir = path.join(projectDir, "dist");
  const webDist = path.join(projectDir, "web", "dist");
  const exePath = findArtifact("SpinningMomo.exe", projectDir);
  const licensePath = path.join(projectDir, "LICENSE");

  if (!fs.existsSync(webDist)) {
    console.error("web/dist not found. Run 'npm run build:web' first.");
    process.exit(1);
  }

  if (!exePath) {
    console.error("SpinningMomo.exe not found under target/. Run 'npm run build:cpp' first.");
    process.exit(1);
  }
  if (!fs.existsSync(licensePath)) {
    console.error("LICENSE not found.");
    process.exit(1);
  }

  console.log(`Preparing ${distDir}...`);

  if (fs.existsSync(distDir)) {
    fs.rmSync(distDir, { recursive: true, force: true });
  }
  fs.mkdirSync(distDir, { recursive: true });

  fs.copyFileSync(exePath, path.join(distDir, "SpinningMomo.exe"));
  fs.copyFileSync(licensePath, path.join(distDir, "LICENSE"));
  fs.cpSync(webDist, path.join(distDir, "resources", "web"), { recursive: true });

  console.log("Done!");
}

main();
