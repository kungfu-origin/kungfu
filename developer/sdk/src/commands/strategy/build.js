const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'build',
  desc: 'Build kungfu strategy',
  run: () => {
    sdk.lib.extension.configure();
    sdk.lib.extension.compile();
  },
};
