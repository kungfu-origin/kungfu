#!/usr/bin/env node

const { getPackageJson, showAllConfig, setAutoConfig, setAutoPrebuiltHost } = require('../lib/shell');

module.exports = require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  cli.command('show', { run: showAllConfig }).command('auto', {
    run: () => {
      const projectJson = getPackageJson();
      setAutoConfig('pypi_mirror');
      [projectJson.binary.module_name, 'link_node'].map(setAutoPrebuiltHost);
      showAllConfig(['@kungfu-trader/libnode']);
    },
  });
});
