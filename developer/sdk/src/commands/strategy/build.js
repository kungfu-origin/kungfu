const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'build',
  desc: 'Build kungfu strategy',
  run: async () => {
    await sdk.lib.extension.installBatch();
    sdk.lib.extension.configure();
    sdk.lib.extension.compile();
  },
};
