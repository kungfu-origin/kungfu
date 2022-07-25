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
  tryRunWithPipenv('poetry', ['python', '-m', 'poetry'], args);
}

function python(args) {
  tryRunWithPipenv('python', ['python'], args);
}

module.exports = require('../lib/sywac')(
  module,
  (cli) => {
    cli
      .command('clear', () => poetry(['cache', 'clear', '--all', 'pypi']))
      .command('lock', {
        setup: (sywac) => {
          sywac
            .option('-q, --quiet', { type: 'boolean', default: false })
            .help();
        },
        run: (argv) => {
          const poetryArgs = argv.quiet ? ['--quiet'] : [];
          poetry(['lock', '-n', '--no-update', ...poetryArgs]);
          python([path.join(shell.getCoreGypDir(), 'format-toml.py')]);
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
