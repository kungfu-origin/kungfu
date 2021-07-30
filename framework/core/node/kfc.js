#!/usr/bin/env node

const path = require('path');
const { spawnSync } = require('child_process');

const kfc_folder = path.join(path.dirname(__dirname), 'dist', 'kfc');

const result = spawnSync('kfc', process.argv.slice(2), {
  stdio: 'inherit',
  shell: process.platform === "win32",
  windowsHide: true,
  env: {
    PATH: `${kfc_folder}${path.delimiter}${process.env.PATH}`,
  },
});

process.exit(result.status);
