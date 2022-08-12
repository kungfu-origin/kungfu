const { getAppDir } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const path = require('path');
const appDir = getAppDir();

module.exports = {
  devRun: require('./dev/run.js'),
  webpackBuild: require('./webpack/build.js'),
  electronBuild: require('./electron/build.js'),
  defaultDistDir: path.join(appDir, 'dist'),
};
