const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

exports.run = function (cmd, argv, check = true) {
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
};
