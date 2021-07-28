const { run } = require('./node-lib.js');
const path = require('path');

function conan(cmd) {
  const pipenv_args = ['run', 'conan', ...cmd];
  run('pipenv', pipenv_args);
}

function getConfigValue(name) {
  return process.env[`npm_package_config_${name}`];
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

exports.argv = require('yargs/yargs')(process.argv.slice(2))
  .command(
    (command = 'install'),
    (description = 'install conan dependencies'),
    (builder = () => {}),
    (handler = () => {
      const settings = makeConanSettings(['build_type']);
      const options = makeConanOptions(['arch', 'log_level', 'freezer', 'node_version', 'electron_version']);
      conan(['install', '.', '-if', 'build', '--build', 'missing', ...settings, ...options]);
    }),
  )
  .command(
    (command = 'build'),
    (description = 'build conan source'),
    (builder = () => {}),
    (handler = () => {
      const settings = makeConanSettings(['build_type']);
      conan(['build', '.', '-bf', 'build', ...settings]);
    }),
  )
  .command(
    (command = 'package'),
    (description = 'package kfc'),
    (builder = () => {}),
    (handler = () => {
      const conanSettings = makeConanSettings(['build_type']);
      conan(['package', '.', '-bf', 'build', '-pf', path.join('dist', 'kfc'), ...conanSettings]);
    }),
  )
  .demandCommand()
  .help().argv;
