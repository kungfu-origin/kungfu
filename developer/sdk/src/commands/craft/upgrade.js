const fse = require('fs-extra');
const path = require('path');

module.exports = {
  flags: 'upgrade',
  desc: 'Upgrade @kungfu-trader packages',
  run: () => {
    sdk.lib.craft.upgrade();
  },
};
