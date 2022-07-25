if (!process.env.CI) {
  const { shell } = require('./framework/core');
  const opts = { silent: true };
  process.env.SKIP_POETRY_LOCK_CHECK = 'on';
  shell.run('yarn', ['-s', 'sync'], true, opts);
  shell.run('yarn', ['-s', 'format'], true, opts);
}
