const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'dev',
  desc: 'Run kungfu artifact in development mode',
  setup: () => {},
  run: () => {
    sdk.lib.craft.dev();
  },
};
