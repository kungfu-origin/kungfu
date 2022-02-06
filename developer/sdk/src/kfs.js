#!/usr/bin/env node

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  cli.commandDirectory('commands');
});
