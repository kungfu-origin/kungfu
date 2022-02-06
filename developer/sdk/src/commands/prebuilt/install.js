const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'install',
  desc: 'Install kungfu oriented project',
  setup: () => {},
  run: () => {
    sdk.lib.prebuilt.install();
  },
};
