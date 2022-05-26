const path = require('path');
const {
  getCliDir,
  getKfcDir,
  getExtensionDirs,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const kfcDir = getKfcDir();
const extdirs = getExtensionDirs();
process.env.KFC_DIR = kfcDir;
process.env.KFC_DEV = true;
process.env.EXTENSION_DIRS = extdirs.join(path.delimiter);

const cliPath = getCliDir();
require(path.resolve(cliPath, 'dist', 'cli', 'index.js'));
