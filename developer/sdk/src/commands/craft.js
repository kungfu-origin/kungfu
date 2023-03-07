const { parseByCli } = require('../utils');

const subcommands = [
  'build',
  'clean',
  'cli',
  'dev',
  'package',
  'publish',
  'upgrade',
];

module.exports = {
  aliases: 'c',
  flags: 'craft <subcommand>',
  ignore: '<subcommand>',
  desc: 'Craft Kungfu Artifact',
  setup: (cli) => {
    if (process.env.NODE_ENV === 'production') {
      subcommands.forEach((subcommand) => {
        const opt = require(`./craft/${subcommand}`);
        cli.command(opt);
      });
    } else {
      cli.commandDirectory('craft');
    }

    parseByCli(cli);
  },
};
