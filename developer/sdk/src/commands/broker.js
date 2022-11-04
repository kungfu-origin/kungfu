const { parseByCli } = require('../utils');

const subcommands = ['build', 'clean', 'package'];

module.exports = {
  aliases: 'b',
  flags: 'broker <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Trading Broker',
  setup: (cli) => {
    if (process.env.NODE_ENV === 'production') {
      subcommands.forEach((subcommand) => {
        const opt = require(`./broker/${subcommand}`);
        cli.command(opt);
      });
    } else {
      cli.commandDirectory('broker');
    }

    parseByCli(cli);
  },
};
