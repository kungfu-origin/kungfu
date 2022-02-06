module.exports = (...argv) => {
  require('../.gyp/node-lib').run('node', [
    require.resolve('@kungfu-trader/kungfu-core/.gyp/node-pre-gyp.js'),
    ...argv,
  ]);
  return '';
};
