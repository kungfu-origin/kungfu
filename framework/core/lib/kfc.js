#!/usr/bin/env node

const path = require('path');
const { kfc } = require('./executable');
const shell = require('./shell');

function getCliDir() {
  try {
    return require('@kungfu-trader/kungfu-js-api/toolkit/utils').getCliDir();
  } catch (err) {
    return '.';
  }
}

shell.run(kfc, process.argv.slice(2), true, {
  silent: true,
  env: {
    KF_CLI_DEV_PATH: path.resolve(getCliDir(), 'lib', 'dev', 'cli.dev.js'),
    KF_LOG_LEVEL: 'trace',
    ...process.env,
  },
});
