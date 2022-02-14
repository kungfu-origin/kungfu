#!/usr/bin/env node

const { spawnSync } = require('child_process');
const executable = require('./executable.js');

const result = spawnSync(executable, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
  env: {
    KF_LOG_LEVEL: 'trace',
    ...process.env,
  },
});

process.exit(result.status);
