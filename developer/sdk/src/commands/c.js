module.exports = {
  flags: 'c <subcommand>',
  ignore: '<subcommand>',
  desc: 'Craft Kungfu Artifact',
  setup: (cli) => cli.commandDirectory('craft'),
};