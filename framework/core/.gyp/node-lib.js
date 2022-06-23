const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const lib = {
  exitOnError: function (error) {
    console.error(error);
    process.exit(-1);
  },
  getConfigValue: function (name) {
    return process.env[`npm_package_config_${name}`];
  },
  runAndCollect: function (cmd, argv = []) {
    return spawnSync(cmd, argv, {
      shell: true,
      stdio: 'pipe',
      windowsHide: true,
    });
  },
  runAndExit: function (cmd, argv = []) {
    const result = lib.run(cmd, argv, false);
    if (result.status === 0) {
      process.exit(0);
    }
    return result;
  },
  run: function (cmd, argv = [], check = true) {
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
    return result;
  },
};

module.exports = lib;
