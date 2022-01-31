const fs = require('fs');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');

const ensureDir = (cwd, dirName) => {
  const distDir = path.join(cwd, dirName);
  fs.mkdirSync(distDir, { recursive: true });
  return distDir;
};

exports.build = async () => {
  const distDir = ensureDir(process.cwd(), 'dist');
  await app.webpackBuild(distDir);
};

exports.package = async () => {
  const buildDir = ensureDir(process.cwd(), 'build');
  await app.electronBuild(buildDir);
};

exports.dev = () => {
  const cwd = process.cwd();
  return app.runDev(ensureDir(cwd, 'dist'));
};
