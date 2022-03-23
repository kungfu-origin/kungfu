const fse = require('fs-extra');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');
const { getAppDir } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const ensureDir = (cwd, ...dirNames) => {
  const targetDir = path.join(cwd, ...dirNames);
  fse.mkdirSync(targetDir, { recursive: true });
  return targetDir;
};

exports.build = () => {
  const appDistDir = path.dirname(
    require.resolve('@kungfu-trader/kungfu-app/dist/app'),
  );
  const cliDistDir = path.dirname(
    require.resolve('@kungfu-trader/kungfu-cli/dist/cli'),
  );
  const targetAppDistDir = ensureDir(process.cwd(), 'dist', 'app');
  const targetCliDistDir = ensureDir(process.cwd(), 'dist', 'cli');

  fse.removeSync(targetAppDistDir);
  fse.removeSync(targetCliDistDir);
  fse.copySync(appDistDir, targetAppDistDir);
  fse.copySync(cliDistDir, targetCliDistDir);
};

exports.package = async () => {
  const buildDir = ensureDir(process.cwd(), 'build');
  await app.electronBuild(buildDir);
};

exports.dev = (withWebpack) => {
  app.devRun(ensureDir(process.cwd(), 'dist'), 'app', withWebpack);
};
