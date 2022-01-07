const { getAppDir } = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const path = require('path');
const appDir = getAppDir();

module.exports = {
    runDev: require('../.build/webpack/dev.js'),
    webpackBuild: require('../.build/webpack/build.js'),
    electronBuild: require('../.build/electron/build.js'),
    defaultDistDir: path.join(appDir, 'dist'),
};
