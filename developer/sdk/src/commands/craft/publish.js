const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'publish',
  desc: 'Package kungfu artifact and publish',
  run: async () => {
    await sdk.lib.craft.package(true);
  },
};
