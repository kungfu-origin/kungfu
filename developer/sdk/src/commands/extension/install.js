const sdk = require('@kungfu-trader/kungfu-sdk');
const os = require('os');

module.exports = {
  flags: 'install',
  desc: 'Install kungfu external dependencies',
  setup: (cli) => {
    cli
      .option('url', {
        type: 'string',
        desc: 'lib site URL',
        defaultValue: sdk.lib.extension.DefaultLibSiteURL,
      })
      .option('lib-name', {
        type: 'string',
        desc: 'lib name',
      })
      .option('lib-version', {
        type: 'string',
        desc: 'lib version',
      })
      .option('arch', {
        type: 'string',
        desc: 'lib arch',
        choices: ['x86', 'x64'],
        defaultValue: os.arch(),
      })
      .option('os', {
        type: 'string',
        desc: 'os platform',
        choices: ['macos', 'linux', 'windows'],
        defaultValue: sdk.lib.extension.detectPlatform(),
      });
  },
  run: async (argv) => {
    if (argv.libName) {
      await sdk.lib.extension.install(
        argv.url,
        argv.libName,
        argv.libVersion,
        argv.os,
        argv.arch,
      );
    } else {
      await sdk.lib.extension.installBatch(argv.url, argv.os, argv.arch);
    }
  },
};
