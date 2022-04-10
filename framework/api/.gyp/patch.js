const path = require('path');
const fs = require('fs-extra');
const { spawnSync } = require('child_process');

const run = (cmd, argv = [], check = true) => {
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
};

try {
  const apiPath = path.dirname(
    require.resolve('@kungfu-trader/kungfu-js-api/package.json'),
  );
  const patchScript = path.resolve(
    apiPath,
    'node_modules',
    '.bin',
    'patch-package',
  );
  run(patchScript);
} catch (err) {
  console.error(err);
}
