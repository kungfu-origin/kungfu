const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu project',
  setup: () => {},
  run: () => {
    sdk.lib.prebuilt.package();
  },
};
