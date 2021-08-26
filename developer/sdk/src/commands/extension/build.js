const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'build',
  desc: 'Build kungfu extensions',
  setup: () => {
  },
  run: () => {
    sdk.lib.extension.build();
  },
};