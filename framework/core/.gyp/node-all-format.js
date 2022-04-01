const path = require('path');
const { spawnSync } = require('child_process');
const moduleDir = path.resolve(
  require.resolve('@kungfu-trader/kungfu-core'),
  '..',
  '..',
);

dealSpawnSyncResult(
  spawnSync('npm', ['run', 'format:cpp'], {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: moduleDir,
  }),
);
dealSpawnSyncResult(
  spawnSync('npm', ['run', 'format:python'], {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: moduleDir,
  }),
);
dealSpawnSyncResult(
  spawnSync('npm', ['run', 'format:js'], {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: moduleDir,
  }),
);

function dealSpawnSyncResult(res) {
  const { stderr, stdout, error } = res;

  if (error) {
    console.error(error);
  }
  if (stdout) {
    console.log(stdout.toString());
  }
  if (stderr) {
    console.error(stderr.toString());
  }
}
