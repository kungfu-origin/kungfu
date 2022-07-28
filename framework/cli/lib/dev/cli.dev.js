const path = require('path');
const {
  getCliDir,
  getKfcDir,
  getExtensionDirs,
  getAppDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const kfcDir = getKfcDir();
const appDir = getAppDir();
const cliDir = getCliDir();
const extdirs = getExtensionDirs();
process.env.KFC_DIR = kfcDir;
process.env.APP_PUBLIC_DIR = path.join(appDir, 'public');
process.env.CLI_DIR = path.join(cliDir, 'dist', 'cli');
process.env.KFC_DEV = true;
process.env.EXTENSION_DIRS = [
  path.join(process.cwd(), 'dist'),
  ...extdirs,
].join(path.delimiter);

const cliPath = getCliDir();
require(path.resolve(cliPath, 'dist', 'cli', 'index.js'));
