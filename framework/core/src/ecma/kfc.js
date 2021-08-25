#!/usr/bin/env node

const path = require('path');
const { spawnSync } = require('child_process');

const kfc_dir = path.join(path.dirname(__dirname), 'dist', 'kfc');
const kfc_bin = path.join(kfc_dir, process.platform === 'win32' ? 'kfc.exe' : 'kfc');

const result = spawnSync(kfc_bin, process.argv.slice(2), {
  stdio: 'inherit',
  windowsHide: true,
});

process.exit(result.status);
