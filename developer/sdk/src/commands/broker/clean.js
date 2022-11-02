const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'clean',
  desc: 'Clean build files',
  run: () => {
    sdk.lib.extension.clean();
  },
};
