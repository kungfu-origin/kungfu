const path = require("path");

module.exports = {
  runDev: require('./dev/run'),
  webpackBuild: require('./webpack/build'),
  electronBuild: require('./electron/build'),
  defaultDistDir: path.join(path.dirname(__dirname), 'dist')
}