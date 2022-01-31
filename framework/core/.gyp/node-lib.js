const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

module.exports = {
  exitOnError: function (error) {
    console.error(error);
    process.exit(-1);
  },
  getConfigValue: function (name) {
    return process.env[`npm_package_config_${name}`];
  },
  run: function (cmd, argv, check = true) {
    const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
    console.log(`$ ${cmd} ${argv.join(' ')}`);
    const result = spawnSync(cmd, argv, {
      shell: true,
      stdio: 'inherit',
      windowsHide: true,
      cwd: real_cwd,
    });
    if (check && result.status !== 0) {
      process.exit(result.status);
    }
  },
};
