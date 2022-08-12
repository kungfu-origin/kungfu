const { prebuilt } = require('@kungfu-trader/kungfu-core');

module.exports = {
  flags: 'install',
  desc: 'Install kungfu prebuilt',
  run: () => {
    prebuilt('install');
  },
};
