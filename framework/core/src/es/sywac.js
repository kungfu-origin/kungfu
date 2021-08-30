module.exports = function(module, setup, opts = {}) {
  const cli = require('sywac');
  const errorHandler = (error) => {
    console.error(error);
    opts.tolerant || process.exit(-1);
  };
  setup(cli);
  opts.silent || cli
    .help('-h, --help')
    .version('-v, --version')
    .showHelpByDefault();
  require.main === module && cli.parseAndExit().catch(errorHandler);
  return cli;
};