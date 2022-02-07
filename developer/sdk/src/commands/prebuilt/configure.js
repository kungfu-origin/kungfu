const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'configure',
  desc: 'Configure kungfu prebuilt',
  setup: (cli) => {
    cli
      .option('write-package-json', {
        type: 'boolean',
        desc: 'write back to package.json',
        defaultValue: false,
      })
      .option('write-workflows', {
        type: 'boolean',
        desc: 'write back to workflows',
        defaultValue: true,
      });
  },
  run: (argv) => {
    sdk.lib.prebuilt.configure(
      argv['write-package-json'],
      argv['write-workflows'],
    );
  },
};
