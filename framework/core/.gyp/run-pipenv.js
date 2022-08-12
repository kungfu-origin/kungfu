const { shell } = require('../lib');

module.exports = require('../lib/sywac')(
  module,
  (cli) => {
    cli
      .option('-q, --quiet', { type: 'boolean', default: false })
      .command('*', {
        setup: (sywac) => {
          sywac.strict(false);
        },
        run: (argv) => {
          const pythonOpt = argv.bare
            ? ['--python', process.env.npm_package_config_python_version]
            : [];
          const pypiOpt = [
            '--pypi-mirror',
            process.env.npm_package_config_pypi_mirror,
          ];
          const cliArgs = process.argv.slice(2);
          const pipenvArgs = [...pythonOpt, ...pypiOpt, ...cliArgs];
          shell.run('pipenv', pipenvArgs);
        },
      });
  },
  { silent: true, help: false, version: false },
);
