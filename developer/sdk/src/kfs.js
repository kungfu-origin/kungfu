#!/usr/bin/env node
const commands = ['craft', 'extension', 'project', 'strategy'];

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  if (process.env.NODE_ENV === 'production') {
    const exitHandler = (result) => {
      if (result.output) {
        console.log(result.output);
        process.exit(result.code);
      }
      if (result.code !== 0) process.exit(result.code);
      if (result.details.args.length === 0) {
        cli.showHelpByDefault().parseAndExit();
      }
      return result.argv;
    };
    const errorHandler = (error) => {
      console.error(error);
      process.exit(-1);
    };
    commands.forEach((command) => {
      const opt = require(`./commands/${command}`);
      cli.command(opt);
    });
    cli.parse().then(exitHandler).catch(errorHandler);
  } else {
    cli.commandDirectory('commands');
  }
});
