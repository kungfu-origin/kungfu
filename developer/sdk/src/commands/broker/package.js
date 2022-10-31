const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu broker',
  run: () => {
    sdk.lib.extension.package();
  },
};
