#!/usr/bin/env node

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  console.log('...');
  cli.commandDirectory('commands');
});
