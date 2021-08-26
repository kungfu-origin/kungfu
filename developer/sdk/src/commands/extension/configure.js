const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'configure',
  desc: 'Configure kungfu extensions',
  setup: () => {},
  run: () => {
    sdk.lib.extension.configure();
  },
};
