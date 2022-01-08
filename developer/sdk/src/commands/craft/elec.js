const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'elec',
  desc: 'elec kungfu artifact',
  setup: () => {},
  run: () => {
    sdk.lib.craft.elec();
  },
};
