const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'clean',
  desc: 'Clean kungfu extensions',
  setup: (cli) => {
    cli.option('keep-libs', {
      type: 'boolean',
      desc: 'Keep downloaded lib files',
      defaultValue: false,
    });
  },
  run: (argv) => {
    sdk.lib.extension.clean(argv.keepLibs);
  },
};
