// SPDX-License-Identifier: Apache-2.0

const { shell } = require('../lib');

function poetry(args) {
  // strip msvc environment variables for cython ext install (i.e. orderedset)
  const env = process.env;
  for (key in process.env) {
    const deleteKeyPrefix = [
      '__',
      'Dev',
      'Framework',
      'npm',
      'VC',
      'VS',
      'Visual',
      'Windows',
    ];
    if (
      deleteKeyPrefix.map((p) => key.startsWith(p)).reduce((a, b) => a || b)
    ) {
      console.log(`-- delete environment variable ${key}`);
      delete env[key];
    }
  }
  console.log(env);
  shell.run('pipenv', ['run', 'python', '-m', 'poetry', ...args], true, {
    env: env,
  });
}

function toPoetryArgs(argv) {
  return argv.quiet ? ['-q', '-n'] : [];
}

function setupPoetryArgs(sywac) {
  sywac.option('-q, --quiet', { type: 'boolean', default: false }).help();
}

function shouldLock() {
  // handle the issue that poetry lock doesn't keep same order across platforms
  const code =
    'from poetry.factory import Factory;' +
    "print(Factory().create_poetry('.').locker.is_fresh())";
  const test = shell.runAndCollect(
    'pipenv',
    ['run', 'python', '-c', `"${code}"`],
    { silent: true },
  );
  return test.out !== 'True';
}

module.exports = require('../lib/sywac')(
  module,
  (cli) => {
    cli
      .command('clear', () => poetry(['cache', 'clear', '--all', 'pypi']))
      .command('lock', {
        setup: setupPoetryArgs,
        run: (argv) =>
          shouldLock() &&
          poetry(['lock', '--no-update', ...toPoetryArgs(argv)]),
      })
      .command('install', {
        setup: setupPoetryArgs,
        run: (argv) => poetry(['install', ...toPoetryArgs(argv)]),
      })
      .command('*', {
        setup: (sywac) => sywac.strict(false),
        run: () => poetry(process.argv.slice(2)),
      });
  },
  { silent: true, help: false, version: false },
);
