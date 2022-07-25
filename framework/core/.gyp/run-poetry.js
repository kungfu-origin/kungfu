const fs = require('fs');
const path = require('path');
const { shell } = require('../lib');

function tryRunWithPipenv(rawCmd, pipenvCmd, args) {
  const enable = !process.env.DISABLE_PIPENV;
  const hasLock = fs.existsSync(path.join(process.cwd(), 'Pipfile.lock'));
  if (enable && hasLock) {
    shell.run('pipenv', ['run', ...pipenvCmd, ...args]);
  } else {
    shell.run(rawCmd, [...args]);
  }
}

function poetry(args) {
  tryRunWithPipenv('poetry', ['python', '-m', 'poetry'], [...args, '-n']);
}

function python(args) {
  tryRunWithPipenv('python', ['python'], args);
}

function toPoetryArgs(argv) {
  return argv.quiet ? ['-q'] : [];
}

function setupPoetryArgs(sywac) {
  sywac
    .option('-q, --quiet', {type: 'boolean', default: false})
    .help();
}

module.exports = require('../lib/sywac')(
  module,
  (cli) => {
    cli
      .command('clear', () => poetry(['cache', 'clear', '--all', 'pypi']))
      .command('lock', {
        setup: setupPoetryArgs,
        run: (argv) => poetry(['lock', '--no-update', ...toPoetryArgs(argv)]),
      })
      .command('install', {
        setup: setupPoetryArgs,
        run: (argv) => {
          poetry(['install', ...toPoetryArgs(argv)]);
          python([path.join(shell.getCoreGypDir(), 'format-toml.py'), 'poetry.lock']);
        },
      })
      .command('*', {
        setup: (sywac) => sywac.strict(false),
        run: () => {
          poetry(process.argv.slice(2));
        },
      });
  },
  { silent: true, help: false, version: false },
);
