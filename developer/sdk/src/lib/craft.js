const fs = require('fs');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');

const ensureDir = (dirName) => {
  const distDir = path.join(process.cwd(), dirName);
  fs.mkdirSync(distDir, { recursive: true });
  return distDir;
};

exports.build = async () => {
  const distDir = ensureDir('dist');
  await app.webpackBuild(distDir);
  await app.electronBuild(distDir);
};

exports.dev = () => {
  return app.runDev(ensureDir('dist'));
};
