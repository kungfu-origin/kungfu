module.exports = {
  executable: require('./executable.js'),
  kungfu: require('./kungfu.js'),
  sywac: require('./sywac.js'),
  project: (...argv) => {
    require('../.gyp/node-lib').run(
      'node',
      [require.resolve('@kungfu-trader/kungfu-core/.gyp/node-pre-gyp.js'), ...argv]
    );
    return '';
  }
};
