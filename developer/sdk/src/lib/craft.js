const fse = require('fs-extra');
const path = require('path');
const { spawnSync } = require('child_process');
const { shell } = require('@kungfu-trader/kungfu-core');

const ensureDir = (cwd, ...dirNames) => {
  const targetDir = path.join(cwd, ...dirNames);
  fse.mkdirSync(targetDir, { recursive: true });
  return targetDir;
};

exports.build = () => {
  const {
    getAppDir,
    getCliDir,
    getKfsDir,
  } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

  const appDistDir = path.join(getAppDir(), 'dist', 'app');
  const publicDir = path.resolve(getAppDir(), 'public');
  const cliDistDir = path.join(getCliDir(), 'dist', 'cli');
  const kfsDistDir = path.join(getKfsDir(), 'dist', 'sdk');

  const targetDistDir = ensureDir(process.cwd(), 'dist');
  const targetAppDistDir = ensureDir(targetDistDir, 'app');
  const targetPublicDistDir = ensureDir(targetDistDir, 'public');
  const targetCliDistDir = ensureDir(targetDistDir, 'cli');
  const targetKfsDistDir = ensureDir(targetDistDir, 'kfs');
  const targetCliDistPublicDir = ensureDir(getCliDir(), 'dist', 'public');

  shell.verifyElectron();

  fse.removeSync(targetDistDir);
  fse.copySync(appDistDir, targetAppDistDir, {});
  fse.copySync(publicDir, targetPublicDistDir, {});
  fse.copySync(cliDistDir, targetCliDistDir, {});
  fse.copySync(kfsDistDir, targetKfsDistDir, {});
  fse.copySync(publicDir, targetCliDistPublicDir, {});
};

exports.package = async () => {
  const buildDir = ensureDir(process.cwd(), 'build');
  await require('@kungfu-trader/kungfu-app').electronBuild(buildDir);
};

exports.dev = (withWebpack) => {
  shell.verifyElectron();
  require('@kungfu-trader/kungfu-app').devRun(
    ensureDir(process.cwd(), 'dist'),
    'app',
    withWebpack,
  );
};

exports.cli = () => {
  const cliPath = require.resolve('@kungfu-trader/kungfu-cli');
  const runExecutable = path.join(cliPath, '..', 'dev', 'cli.dev.js');
  spawnSync('node', [runExecutable, ...process.argv.slice(4)], {
    stdio: 'inherit',
    windowsHide: true,
  });
};

exports.upgrade = () => {
  shell.run('yarn upgrade', ['--scope', '@kungfu-trader']);
};
