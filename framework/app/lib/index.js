const { getAppDir } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const path = require('path');
const appDir = getAppDir();

module.exports = {
  runDev: require('./webpack/dev.js'),
  webpackBuild: require('./webpack/build.js'),
  electronBuild: require('./electron/build.js'),
  defaultDistDir: path.join(appDir, 'dist'),
};
