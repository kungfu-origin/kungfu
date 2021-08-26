const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'clean',
  desc: 'Clean kungfu extensions',
  setup: (cli) => {
    yargs.option('keep-libs', {
      type: 'boolean',
      desc: 'Keep downloaded lib files',
      defaultValue: true,
    });
  },
  run: (argv) => {
    sdk.lib.extension.clean(argv.keepLibs);
  },
};
