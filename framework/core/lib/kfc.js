#!/usr/bin/env node

const { getCliDir } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const { spawnSync } = require('child_process');
const executable = require('./executable.js');
const path = require('path');

const result = spawnSync(executable, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
  env: {
    KF_CLI_DEV_PATH: path.resolve(getCliDir(), 'lib', 'dev', 'devCli.js'),
    KF_LOG_LEVEL: 'trace',
    ...process.env,
  },
});

process.exit(result.status);
