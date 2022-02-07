const fse = require('fs-extra');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');

const ensureDir = (cwd, ...dirNames) => {
  const targetDir = path.join(cwd, ...dirNames);
  fse.mkdirSync(targetDir, { recursive: true });
  return targetDir;
};

exports.build = () => {
  const appDrone = '@kungfu-trader/kungfu-app/dist/app/kungfu-app.node';
  const srcDir = path.dirname(require.resolve(appDrone));
  const targetDir = ensureDir(process.cwd(), 'dist', 'app');
  fse.removeSync(targetDir);
  fse.copySync(srcDir, targetDir);
};

exports.package = async () => {
  const buildDir = ensureDir(process.cwd(), 'build');
  await app.electronBuild(buildDir);
};

exports.dev = (withWebpack) => {
  app.devRun(ensureDir(process.cwd(), 'dist'), 'app', withWebpack);
};
