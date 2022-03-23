const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'cli',
  desc: 'Run kungfu artifact in cli mode',
  run: (argv) => {
    sdk.lib.craft.cli(argv);
  },
};
