module.exports = {
  aliases: 'c',
  flags: 'craft <subcommand>',
  ignore: '<subcommand>',
  desc: 'Craft Kungfu Artifact',
  setup: (cli) => cli.commandDirectory('craft'),
};
