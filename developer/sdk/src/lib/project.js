const path = require('path');

exports.install = () => {
  const cli = require('@kungfu-trader/kungfu-core/.gyp/node-pre-gyp');
  cli.parseAndExit().catch(console.error);
};

exports.configure = (writePackageJson = false, writeWorkflows = true) => {
  const config = require(path.join(process.cwd(), 'package.json'));
  if (writePackageJson) {
    console.log('> writing package.json');
    console.log(JSON.stringify(config, null, 2));
  }
  if (writeWorkflows) {
    console.log('> writing workflows');
  }
};
