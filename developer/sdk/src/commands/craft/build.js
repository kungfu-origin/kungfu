const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'build',
  desc: 'Build kungfu artifact',
  run: () => {
    sdk.lib.craft.build();
  },
};
