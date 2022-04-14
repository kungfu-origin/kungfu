module.exports = {
  aliases: 'p',
  flags: 'project <subcommand>',
  ignore: '<subcommand>',
  desc: 'Manage Kungfu Projects',
  setup: (cli) => cli.commandDirectory('project'),
};
