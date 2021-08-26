module.exports = {
  flags: 'x <subcommand>',
  ignore: '<subcommand>',
  desc: 'Build Kungfu Extensions',
  setup: (cli) => cli.commandDirectory('extension'),
};