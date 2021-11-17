const fs = require('fs');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');

const ensureDistDir = () => {
  const distDir = path.join(process.cwd(), 'dist');
  fs.mkdirSync(distDir, { recursive: true });
  return distDir;
};

exports.build = async () => {
  const distDir = ensureDistDir();
  await app.webpackBuild(distDir);
  await app.electronBuild(distDir);
};

exports.dev = () => {
  return app.runDev(ensureDistDir());
};
