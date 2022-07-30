const subcommands = ['build', 'clean', 'package'];

module.exports = {
  aliases: 's',
  flags: 'strategy <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Trading Strategy',
  setup: (cli) => {
    if (process.env.NODE_ENV === 'production') {
      subcommands.forEach((subcommand) => {
        const opt = require(`./strategy/${subcommand}`);
        cli.command(opt);
      });
    } else {
      cli.commandDirectory('strategy');
    }
  },
};
