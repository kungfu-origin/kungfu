const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'package',
  desc: 'Package kungfu strategy',
  run: () => {
    sdk.lib.project.package();
  },
};
