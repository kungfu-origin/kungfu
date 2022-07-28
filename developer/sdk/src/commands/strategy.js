module.exports = {
  aliases: 's',
  flags: 'strategy <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Trading Strategy',
  setup: (cli) => cli.commandDirectory('strategy'),
};
