const sdk = require('@kungfu-trader/kungfu-sdk');
const treeify = require('treeify');

module.exports = {
  flags: 'list',
  desc: 'List kungfu external dependencies',
  setup: (cli) => {
    cli
      .option('url', {
        type: 'string',
        desc: 'lib site URL',
        defaultValue: sdk.lib.extension.DefaultLibSiteURL,
      })
      .option('match-name', {
        type: 'string',
        desc: 're expression to filter lib name',
        defaultValue: '.*',
      })
      .option('match-version', {
        type: 'string',
        desc: 're expression to filter lib version',
        defaultValue: '.*',
      })
      .option('list-version', {
        type: 'boolean',
        desc: 'show lib versions',
        defaultValue: true,
      })
      .option('list-platform', {
        type: 'boolean',
        desc: 'show platform implementaions',
        defaultValue: false,
      });
  },
  run: async (argv) => {
    const libs = await sdk.lib.extension.list(
      argv.url,
      argv.matchName,
      argv.matchVersion,
      argv.listVersion,
      argv.listPlatform,
    );
    console.log(treeify.asTree(libs, true));
  },
};
