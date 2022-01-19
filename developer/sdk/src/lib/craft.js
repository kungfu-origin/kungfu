const fs = require('fs');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');

const ensureDir = (cwd, dirName) => {
  const distDir = path.join(cwd, dirName);
  fs.mkdirSync(distDir, { recursive: true });
  return distDir;
};

const runWebpackBuild = async (cwd) => {
  const distDir = ensureDir(cwd, 'dist');
  await app.webpackBuild(distDir);
};

const runElectronBuild = async (cwd) => {
  const buildDir = ensureDir(cwd, 'build');
  await app.electronBuild(buildDir);
};

exports.build = async () => {
  const cwd = process.cwd();
  await runWebpackBuild(cwd);
  await runElectronBuild(cwd);
};

exports.dev = () => {
  const cwd = process.cwd();
  return app.runDev(ensureDir(cwd, 'dist'));
};
