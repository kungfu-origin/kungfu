#!/usr/bin/env node
const commands = ['craft', 'extension', 'project', 'strategy'];

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  if (process.env.NODE_ENV === 'production') {
    commands.forEach((command) => {
      const opt = require(`./commands/${command}`);
      cli.command(opt);
    });
  } else {
    cli.commandDirectory('commands');
  }
});
