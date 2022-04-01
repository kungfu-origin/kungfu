const { run } = require('./node-lib.js');

dealSpawnSyncResult(run('yarn', ['format:cpp']));
dealSpawnSyncResult(run('yarn', ['format:python']));
dealSpawnSyncResult(run('yarn', ['format:js']));

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
