module.exports = function (module, setup, opts = {}) {
  const cli = require('sywac').strict();
  const errorHandler = (error) => {
    console.error(error);
    opts.tolerant || process.exit(-1);
  };
  setup(cli);
  opts.noHelp || cli.help('-h, --help');
  opts.noVersion || cli.version('-v, --version');
  require.main === module && cli.parse().then(result => {
    if (result.output) {
      console.log(result.output)
      process.exit(result.code)
    }
    if (result.code !== 0) process.exit(result.code)
    if (!opts.silent) {
      cli.showHelpByDefault().parseAndExit();
    }
    return result.argv
  }).catch(errorHandler);
  return cli;
};
