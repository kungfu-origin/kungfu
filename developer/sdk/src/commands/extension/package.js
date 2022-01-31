const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu extensions',
  setup: () => {},
  run: () => {
    sdk.lib.extension.package();
  },
};
