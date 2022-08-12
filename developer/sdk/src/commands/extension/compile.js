const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'compile',
  desc: 'Compile kungfu extensions',
  run: () => {
    sdk.lib.extension.compile();
  },
};
