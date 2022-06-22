const { run } = require('./node-lib.js');

try {
  dealSpawnSyncResult(run('yarn', ['configure']));
  dealSpawnSyncResult(run('yarn', ['format:cpp']));
  dealSpawnSyncResult(run('yarn', ['format:python']));
  dealSpawnSyncResult(run('yarn', ['format:js']));
} catch (err) {
  console.error(err);
}

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
