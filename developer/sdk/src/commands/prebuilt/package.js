const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu prebuilt',
  setup: () => {},
  run: () => {
    sdk.lib.prebuilt.package();
  },
};
