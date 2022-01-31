#!/usr/bin/env node

const { spawnSync, exec } = require('child_process');
const executable = require('./executable.js');

const result = spawnSync(executable, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
});

process.exit(result.status);
