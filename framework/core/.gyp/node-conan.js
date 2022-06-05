const { exitOnError, getConfigValue, run } = require('./node-lib.js');
const fse = require('fs-extra');
const path = require('path');

function conan(cmd) {
  const pipenv_args = ['run', 'conan', ...cmd];
  run('pipenv', pipenv_args);
}

function getNodeVersionOptions() {
  const packageJson = fse.readJsonSync(
    path.resolve(path.dirname(__dirname), 'package.json'),
  );
  const electronVersion = packageJson.dependencies['electron'];
  const nodeVersion = packageJson.devDependencies['@kungfu-trader/libnode'];
  return [
    '-o',
    `electron_version=${electronVersion}`,
    '-o',
    `node_version=${nodeVersion}`,
  ];
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
  return names.map(makeConanOption).flat().concat(getNodeVersionOptions());
}

function conanInstall() {
  const settings = makeConanSettings(['build_type']);
  const options = makeConanOptions(['arch', 'log_level', 'freezer']);
  conan([
    'install',
    '.',
    '-if',
    'build',
    '--build',
    'missing',
    ...settings,
    ...options,
  ]);
}

function conanBuild() {
  const settings = makeConanSettings(['build_type']);
  conan(['build', '.', '-bf', 'build', ...settings]);
}

function conanBuildOnGitHub() {
  // Have to use Clang to build pykungfu because MSVC suffers to OutOfMemoery error

  const resetBuild = () => fse.removeSync(path.join('build', 'CMakeCache.txt'));

  // Build pykungfu with Clang
  process.env.CONAN_VS_TOOLSET = 'ClangCL';
  process.env.KUNGFU_BUILD_SKIP_RUNTIME_ELECTRON = true;
  delete process.env.KUNGFU_BUILD_SKIP_RUNTIME_NODE;
  resetBuild();
  conanInstall();
  conanBuild();

  // Build kungfu_node with MSVC
  process.env.CONAN_VS_TOOLSET = 'auto';
  process.env.KUNGFU_BUILD_SKIP_RUNTIME_NODE = true;
  delete process.env.KUNGFU_BUILD_SKIP_RUNTIME_ELECTRON;
  resetBuild();
  conanInstall();
  conanBuild();
}

function conanPackage() {
  const conanSettings = makeConanSettings(['build_type']);
  conan([
    'package',
    '.',
    '-bf',
    'build',
    '-pf',
    path.join('dist', 'kfc'),
    ...conanSettings,
  ]);
}

const cli = require('sywac')
  .command('install', {
    run: conanInstall,
  })
  .command('build', {
    run: () =>
      process.env.GITHUB_ACTIONS && process.platform == 'win32'
        ? conanBuildOnGitHub()
        : conanBuild(),
  })
  .command('package', {
    run: conanPackage,
  })
  .help('--help')
  .version('--version')
  .showHelpByDefault();

module.exports = cli;

async function main() {
  await cli.parseAndExit();
}

if (require.main === module) main().catch(exitOnError);
