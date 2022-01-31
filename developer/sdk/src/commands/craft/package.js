const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu artifact',
  setup: () => {},
  run: () => {
    sdk.lib.craft.package();
  },
};
