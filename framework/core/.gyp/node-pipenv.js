const { exitOnError, run } = require('./node-lib.js');

const cli = require('sywac')
  // options
  .option('bare', { type: 'boolean', default: false })
  .help('--help')
  .version('--version');

module.exports = cli;

async function main() {
  const argv = await cli.parseAndExit();

  const bare_opt = argv.bare ? ['--bare'] : [];
  const python_opt = argv.bare ? ['--python', process.env.npm_package_config_python_version] : [];
  const pypi_opt = ['--pypi-mirror', process.env.npm_package_config_pypi_mirror];
  const pipenv_args = [...bare_opt, ...python_opt, ...pypi_opt, ...argv._];

  run('pipenv', pipenv_args);
}

if (require.main === module) main().catch(exitOnError);
