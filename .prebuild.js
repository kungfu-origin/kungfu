const { run } = require('./framework/core/.gyp/node-lib.js');

if (!process.env.CI) {
  run('yarn', ['sync']);
  run('yarn', ['format']);
}
