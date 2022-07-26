if (!process.env.CI) {
  const { shell } = require('./framework/core');
  const opts = { silent: true };
  shell.run('yarn', ['-s', 'sync'], true, opts);
  shell.run('yarn', ['-s', 'format'], true, opts);
}
