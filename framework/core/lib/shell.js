const { spawnSync } = require('child_process');
const fs = require('fs');
const fse = require('fs-extra');
const path = require('path');

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };

const scope = (npmConfigValue) =>
  npmConfigValue === 'undefined' ? '[package.json]' : '[user]';

const getPackageJson = (module) => {
  if (!module) {
    return fse.readJsonSync(
      path.resolve(path.dirname(__dirname), 'package.json'),
    );
  }
  try {
    return fse.readJsonSync(require.resolve(`${module}/package.json`));
  } catch (e) {
    return undefined;
  }
};

const getNpmConfigValue = (key) => {
  return spawnSync('npm', ['config', 'get', key], spawnOptsPipe)
    .output.filter((e) => e && e.length > 0)
    .toString()
    .trimEnd();
};

const exitOnError = (error) => {
  console.error(error);
  process.exit(-1);
};

const getConfigValue = (name) => {
  return process.env[`npm_package_config_${name}`];
};

const isGithubEnv = () => true;//process.env.CI && process.env.GITHUB_ACTIONS;

const npmCall = (npmArgs) => {
  console.log(`$ npm ${npmArgs.join(' ')}`);
  const result = spawnSync('npm', npmArgs, spawnOptsInherit);
  if (result.status !== 0) {
    console.error(`Failed with status ${result.status}`);
    process.exit(result.status);
  }
};

const run = (cmd, argv = [], check = true, opts = {}) => {
  const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
  console.log(`$ ${cmd} ${argv.join(' ')}`);
  const result = spawnSync(cmd, argv, {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: real_cwd,
    ...opts,
  });
  if (check && result.status !== 0) {
    process.exit(result.status);
  }
  return result;
};

const runAndCollect = (cmd, argv = [], opts = {}) => {
  return spawnSync(cmd, argv, {
    shell: true,
    stdio: 'pipe',
    windowsHide: true,
    ...opts,
  });
};

const runAndExit = (cmd, argv = [], opts = {}) => {
  const result = run(cmd, argv, false, opts);
  if (result.status === 0) {
    process.exit(0);
  }
  return result;
};

const setAutoConfig = (key, globalScope = false) => {
  if (isGithubEnv()) {
    const packageJson = getPackageJson();
    npmCall([
      'config',
      'set',
      `${packageJson.name}:${key}`,
      packageJson.config[`${key}_github`],
    ]);
  }
};

const setAutoPrebuiltHost = (module) => {
  if (isGithubEnv()) {
    const packageJson = getPackageJson();
    const key = `${module}_binary_host_mirror`;
    npmCall(['config', 'set', key, packageJson.config['binary_host_github']]);
  }
};

const showProjectConfig = (key) => {
  const packageJson = getPackageJson();
  const projectName = packageJson.name;
  const npmConfigKey = `${projectName}:${key}`;
  const npmConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    npmConfigValue === 'undefined' ? packageJson.config[key] : npmConfigValue;
  console.log(`[config] ${npmConfigKey} = ${value} ${scope(npmConfigValue)}`);
};

const showPrebuiltHostConfig = (module) => {
  const packageJson = getPackageJson(module);
  const key = packageJson.binary.module_name;
  const npmConfigKey = `${key}_binary_host_mirror`;
  const hostConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    hostConfigValue === 'undefined' && packageJson
      ? packageJson.binary.host
      : hostConfigValue;
  console.log(`[binary] ${npmConfigKey} = ${value} ${scope(hostConfigValue)}`);
};

const showAllConfig = (modules) => {
  const packageJson = getPackageJson();
  Object.keys(packageJson.config).map(showProjectConfig);
  if (packageJson.binary) {
    showPrebuiltHostConfig();
  }
  modules.map(showPrebuiltHostConfig);
};

module.exports = {
  getPackageJson: getPackageJson,
  exitOnError: exitOnError,
  getConfigValue: getConfigValue,
  npmCall: npmCall,
  run: run,
  runAndCollect: runAndCollect,
  runAndExit: runAndExit,
  setAutoConfig: setAutoConfig,
  setAutoPrebuiltHost: setAutoPrebuiltHost,
  showAllConfig: showAllConfig,
};
