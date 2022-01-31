#!/usr/bin/env node

require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  cli.commandDirectory('commands');
});
