const { shell } = require('../lib');

try {
  dealSpawnSyncResult(shell.run('yarn', ['format:cpp']));
  dealSpawnSyncResult(shell.run('yarn', ['format:python']));
  dealSpawnSyncResult(shell.run('yarn', ['format:js']));
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
