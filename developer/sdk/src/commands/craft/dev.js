const sdk = require('@kungfu-trader/kungfu-sdk');

module.exports = {
  flags: 'dev',
  desc: 'Run kungfu artifact in development mode',
  setup: (cli) => {
    cli.option('with-webpack', {
      type: 'boolean',
      desc: 'run webpack dev server',
      defaultValue: false,
    });
  },
  run: (argv) => {
    sdk.lib.craft.dev(argv['with-webpack']);
  },
};
