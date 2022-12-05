// SPDX-License-Identifier: Apache-2.0

module.exports = function (module, setup, opts = {}) {
  const cli = require('sywac').strict();
  const exitHandler = (result) => {
    if (result.output) {
      console.log(result.output);
      process.exit(result.code);
    }
    if (result.code !== 0) process.exit(result.code);
    if (!opts.silent && result.details.args.length === 0) {
      cli.showHelpByDefault().parseAndExit();
    }
    return result.argv;
  };
  const errorHandler = (error) => {
    console.error(error);
    opts.tolerant || process.exit(-1);
  };
  setup(cli);
  if (opts.help === undefined || opts.help) {
    cli.help('-h, --help');
  }
  if (opts.version === undefined || opts.version) {
    cli.version('-v, --version');
  }
  if (require.main === module) {
    cli.parse().then(exitHandler).catch(errorHandler);
  }
  return cli;
};
