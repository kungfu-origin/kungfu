#!/usr/bin/env node

const { spawnSync } = require('child_process');
const executable = require('./executable.js');
const path = require('path');

function getCliDir() {
  try {
    return require('@kungfu-trader/kungfu-js-api/toolkit/utils').getCliDir();
  } catch (err) {
    return '.';
  }
}

const result = spawnSync(executable, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
  env: {
    KF_CLI_DEV_PATH: path.resolve(getCliDir(), 'lib', 'dev', 'cli.dev.js'),
    KF_LOG_LEVEL: 'trace',
    ...process.env,
  },
});

process.exit(result.status);
