const { exitOnError, getConfigValue, run } = require('./node-lib.js');
const path = require('path');

function conan(cmd) {
  const pipenv_args = ['run', 'conan', ...cmd];
  run('pipenv', pipenv_args);
}

function makeConanSetting(name) {
  return ['-s', `${name}=${getConfigValue(name)}`];
}

function makeConanSettings(names) {
  return names.map(makeConanSetting).flat();
}

function makeConanOption(name) {
  return ['-o', `${name}=${getConfigValue(name)}`];
}

function makeConanOptions(names) {
  return names.map(makeConanOption).flat();
}

const cli = require('sywac')
  .command('install', {
    run: () => {
      const settings = makeConanSettings(['build_type']);
      const options = makeConanOptions(['arch', 'log_level', 'freezer', 'node_version', 'electron_version']);
      conan(['install', '.', '-if', 'build', '--build', 'missing', ...settings, ...options]);
    },
  })
  .command('build', {
    run: () => {
      const settings = makeConanSettings(['build_type']);
      conan(['build', '.', '-bf', 'build', ...settings]);
    },
  })
  .command('package', {
    run: () => {
      const conanSettings = makeConanSettings(['build_type']);
      conan(['package', '.', '-bf', 'build', '-pf', path.join('dist', 'kfc'), ...conanSettings]);
    },
  })
  .help('--help')
  .version('--version')
  .showHelpByDefault();

module.exports = cli;

async function main() {
  await cli.parseAndExit();
}

if (require.main === module) main().catch(exitOnError);
