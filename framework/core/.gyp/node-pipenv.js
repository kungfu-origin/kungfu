const { shell } = require('../lib');

const cli = require('sywac')
  // options
  .option('bare', { type: 'boolean', default: false })
  .help('--help')
  .version('--version');

async function main() {
  const argv = await cli.parseAndExit();

  const bare_opt = argv.bare ? ['--bare'] : [];
  const python_opt = argv.bare
    ? ['--python', process.env.npm_package_config_python_version]
    : [];
  const pypi_opt = [
    '--pypi-mirror',
    process.env.npm_package_config_pypi_mirror,
  ];
  const pipenv_args = [...bare_opt, ...python_opt, ...pypi_opt, ...argv._];

  shell.run('pipenv', pipenv_args);
}

module.exports.cli = cli;
module.exports.main = main;

if (require.main === module) main().catch(shell.exitOnError);
