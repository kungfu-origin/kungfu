#!/usr/bin/env node

const path = require('path');
const { spawnSync } = require('child_process');

const kfc_folder = path.join(path.dirname(__dirname), "build", "kfc")

const result = spawnSync('kfc', process.argv.slice(2), {
  stdio: 'inherit',
  shell: true,
  windowsHide: true,
  env: {
    PATH: kfc_folder
  }
})

process.exit(result.status)
