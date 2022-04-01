const { spawnSync } = require('child_process');

dealSpawnSyncResult(spawnSync('yarn', ['format:cpp']));
dealSpawnSyncResult(spawnSync('yarn', ['format:python']));
dealSpawnSyncResult(spawnSync('yarn', ['format:js']));

function dealSpawnSyncResult(res) {
  const { stderr, stdout } = res;
  console.log(stdout.toString());
  console.error(stderr.toString());
}
