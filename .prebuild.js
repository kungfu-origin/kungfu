if (!process.env.CI) {
  const { shell } = require('./framework/core');
  shell.run('yarn', ['sync']);
  shell.run('yarn', ['format'], false);
}
