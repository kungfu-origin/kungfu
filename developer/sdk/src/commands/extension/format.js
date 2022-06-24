const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'format',
  desc: 'Format kungfu extension code',
  run: () => {
    sdk.lib.extension.format();
  },
};
