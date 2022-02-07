module.exports = {
  aliases: 'p',
  flags: 'prebuilt <subcommand>',
  ignore: '<subcommand>',
  desc: 'Manage Kungfu Prebuilt',
  setup: (cli) => cli.commandDirectory('prebuilt'),
};
