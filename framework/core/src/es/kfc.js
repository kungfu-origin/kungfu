#!/usr/bin/env node

const path = require('path');
const { spawnSync } = require('child_process');

const kfcDir = path.resolve(__dirname, '..', '..', 'dist', 'kfc');
const kfcBin = path.resolve(kfcDir, process.platform === 'win32' ? 'kfc.exe' : 'kfc');

const result = spawnSync(kfcBin, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
});

process.exit(result.status);
