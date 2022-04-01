const { spawnSync } = require('child_process');

dealSpawnSyncResult(spawnSync('yarn', ['format:cpp']));
dealSpawnSyncResult(spawnSync('yarn', ['format:python']));
dealSpawnSyncResult(spawnSync('yarn', ['format:js']));

function dealSpawnSyncResult(res) {
  const { stderr, stdout } = res;
  if (stdout) {
    console.log(stdout.toString());
  }
  if (stderr) {
    console.error(stderr.toString());
  }
}
