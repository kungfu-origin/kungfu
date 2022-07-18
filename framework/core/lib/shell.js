const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

module.exports.exitOnError = function (error) {
  console.error(error);
  process.exit(-1);
};

module.exports.getConfigValue = function (name) {
  return process.env[`npm_package_config_${name}`];
};

module.exports.runAndCollect = function (cmd, argv = [], opts = {}) {
  return spawnSync(cmd, argv, {
    shell: true,
    stdio: 'pipe',
    windowsHide: true,
    ...opts,
  });
};

module.exports.runAndExit = function (cmd, argv = [], opts = {}) {
  const result = module.exports.run(cmd, argv, false, opts);
  if (result.status === 0) {
    process.exit(0);
  }
  return result;
};

module.exports.run = function (cmd, argv = [], check = true, opts = {}) {
  const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
  console.log(`$ ${cmd} ${argv.join(' ')}`);
  const result = spawnSync(cmd, argv, {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: real_cwd,
    ...opts,
  });
  if (check && result.status !== 0) {
    process.exit(result.status);
  }
  return result;
};
