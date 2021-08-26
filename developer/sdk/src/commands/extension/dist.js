const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'dist',
  desc: 'Dist kungfu extensions',
  setup: () => {
  },
  run: () => {
    sdk.lib.extension.dist();
  },
};