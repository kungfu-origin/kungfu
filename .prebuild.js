if (!process.env.CI) {
  const { run } = require('./framework/core/.gyp/node-lib.js');
  run('yarn', ['sync']);
  run('yarn', ['format'], false);
}
