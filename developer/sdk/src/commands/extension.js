const { parseByCli } = require('../utils');

const subcommands = [
  'build',
  'clean',
  'compile',
  'configure',
  'format',
  'install',
  'list',
];

module.exports = {
  aliases: 'x',
  flags: 'extension <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Extensions',
  setup: (cli) => {
    if (process.env.NODE_ENV === 'production') {
      subcommands.forEach((subcommand) => {
        const opt = require(`./extension/${subcommand}`);
        cli.command(opt);
      });
    } else {
      cli.commandDirectory('extension');
    }

    parseByCli(cli);
  },
};
