const { spawnSync } = require('child_process');

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

  console.log(`$ pipenv ${pipenv_args.join(' ')}`);

  const result = spawnSync('pipenv', pipenv_args, {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
  });

  process.exit(result.status);
}

if (require.main === module) main();
