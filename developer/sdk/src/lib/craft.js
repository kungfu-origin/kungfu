const fse = require('fs-extra');
const path = require('path');
const { spawnSync } = require('child_process');
const { shell } = require('@kungfu-trader/kungfu-core');
const { customResolve } = require('../utils');

const ensureDir = (cwd, ...dirNames) => {
  const targetDir = path.join(cwd, ...dirNames);
  fse.mkdirSync(targetDir, { recursive: true });
  return targetDir;
};

exports.build = () => {
  const {
    getAppDir,
    getCliDir,
    getSdkDir,
    getJsApi,
  } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

  const appDistDir = path.join(getAppDir(), 'dist', 'app');
  const publicDir = path.join(getAppDir(), 'public');
  const cliDistDir = path.join(getCliDir(), 'dist', 'cli');
  const apiDistDir = path.join(getJsApi(), 'dist', 'api');
  const kfsDistDir = path.join(getSdkDir(), 'dist', 'sdk');

  const targetDistDir = ensureDir(process.cwd().toString(), 'dist');
  const targetAppDistDir = ensureDir(targetDistDir, 'app');
  const targetPublicDistDir = ensureDir(targetDistDir, 'public');
  const targetCliDistDir = ensureDir(targetDistDir, 'cli');
  const targetApiDistDir = ensureDir(targetDistDir, 'api');
  const targetKfsDistDir = ensureDir(targetDistDir, 'kfs');
  const targetCliDistPublicDir = ensureDir(getCliDir(), 'dist', 'public');

  shell.verifyElectron();

  fse.removeSync(targetDistDir);
  fse.copySync(appDistDir, targetAppDistDir, {});
  fse.copySync(publicDir, targetPublicDistDir, {});
  fse.copySync(cliDistDir, targetCliDistDir, {});
  fse.copySync(apiDistDir, targetApiDistDir, {});
  fse.copySync(kfsDistDir, targetKfsDistDir, {});
  fse.copySync(publicDir, targetCliDistPublicDir, {});
};

exports.package = async (andPublish = false) => {
  const buildDir = ensureDir(process.cwd().toString(), 'build');
  await require('@kungfu-trader/kungfu-app').electronBuild(
    buildDir,
    andPublish,
  );
};

exports.dev = (withWebpack) => {
  shell.verifyElectron();
  require('@kungfu-trader/kungfu-app').devRun(
    ensureDir(process.cwd().toString(), 'dist'),
    'app',
    withWebpack,
  );
};

exports.cli = () => {
  const cliPath = customResolve('@kungfu-trader/kungfu-cli');
  const runExecutable = path.join(cliPath, '..', 'dev', 'cli.dev.js');
  spawnSync('node', [runExecutable, ...process.argv.slice(4)], {
    stdio: 'inherit',
    windowsHide: true,
  });
};

exports.upgrade = () => {
  shell.run('yarn upgrade', ['--scope', '@kungfu-trader']);
};
