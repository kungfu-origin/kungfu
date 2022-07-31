'use strict';

const { shell, utils } = require('@kungfu-trader/kungfu-js-api/toolkit');

function start(argv, name) {
  return new Promise((resolve) => {
    const config = require(`../webpack/webpack.config`)(argv);
    const compiler = require('webpack')(config);
    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      shell.logStats(name, stats);
    });

    compiler.hooks.done.tap('components-compile-done', (stats) => {
      shell.logStats(name, stats);
      resolve();
    });
  });
}

const run = (distDir, distName = 'cli') => {
  shell.greeting();

  const argv = utils.buildDevArgv(distDir, distName);
  const cliDir = utils.getCliDir();

  process.chdir(cliDir);

  return Promise.all([start(argv, 'sdk')]);
};

module.exports = run;

if (require.main === module) {
  shell.runDist(utils.getCliDefaultDistDir(), run);
}
