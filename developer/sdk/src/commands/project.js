const { parseByCli } = require('../utils');

const subcommands = ['configure', 'install', 'package'];

module.exports = {
  aliases: 'p',
  flags: 'project <subcommand>',
  ignore: '<subcommand>',
  desc: 'Manage Kungfu Projects',
  setup: (cli) => {
    if (process.env.NODE_ENV === 'production') {
      subcommands.forEach((subcommand) => {
        const opt = require(`./project/${subcommand}`);
        cli.command(opt);
      });
    } else {
      cli.commandDirectory('project');
    }

    parseByCli(cli);
  },
};
