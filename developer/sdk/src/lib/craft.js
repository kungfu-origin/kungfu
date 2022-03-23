const fse = require('fs-extra');
const path = require('path');
const app = require('@kungfu-trader/kungfu-app');
const {
  getAppDir,
  getCliDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const { spawnSync } = require('child_process');

const ensureDir = (cwd, ...dirNames) => {
  const targetDir = path.join(cwd, ...dirNames);
  fse.mkdirSync(targetDir, { recursive: true });
  return targetDir;
};

exports.build = () => {
  const appDistDir = path.join(getAppDir(), 'dist', 'app');
  const publicDir = path.resolve(getAppDir(), 'public');
  const cliDistDir = path.join(getCliDir(), 'dist', 'cli');

  const targetDistDir = ensureDir(process.cwd(), 'dist');
  const targetAppDistDir = ensureDir(targetDistDir, 'app');
  const targetPublicDistDir = ensureDir(targetDistDir, 'public');
  const targetCliDistDir = ensureDir(targetDistDir, 'cli');

  fse.removeSync(targetDistDir);
  fse.copySync(appDistDir, targetAppDistDir);
  fse.copySync(publicDir, targetPublicDistDir);
  fse.copySync(cliDistDir, targetCliDistDir);
};

exports.package = async () => {
  const buildDir = ensureDir(process.cwd(), 'build');
  await app.electronBuild(buildDir);
};

exports.dev = (withWebpack) => {
  app.devRun(ensureDir(process.cwd(), 'dist'), 'app', withWebpack);
};

exports.cli = () => {
  const cliPath = require.resolve('@kungfu-trader/kungfu-cli');
  const runExecutable = path.join(cliPath, '..', 'dev', 'devCli.js');

  spawnSync('node', [runExecutable, ...process.argv.slice(4)], {
    stdio: 'inherit',
    windowsHide: true,
  });
};
