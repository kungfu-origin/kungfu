module.exports = {
  aliases: 'x',
  flags: 'extension <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Extensions',
  setup: (cli) => cli.commandDirectory('extension'),
};
