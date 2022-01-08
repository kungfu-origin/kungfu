const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'dist',
  desc: 'dist kungfu artifact',
  setup: () => {},
  run: () => {
    sdk.lib.craft.dist();
  },
};
