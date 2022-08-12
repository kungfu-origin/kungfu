const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'upgrade',
  desc: 'Upgrade @kungfu-trader packages',
  run: () => {
    sdk.lib.craft.upgrade();
  },
};
