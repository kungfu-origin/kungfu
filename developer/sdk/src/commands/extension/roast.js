const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'roast',
  desc: 'Make best try for building extension',
  setup: (cli) => {
    cli.option('url', {
      type: 'string',
      desc: 'lib site URL',
      defaultValue: sdk.lib.extension.DefaultLibSiteURL,
    });
  },
  run: async (argv) => {
    await sdk.lib.extension.installBatch(argv.url);
    sdk.lib.extension.configure();
    sdk.lib.extension.build();
  },
};
