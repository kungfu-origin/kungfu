const { spawnSync } = require('child_process');
const fs = require('fs');
const fse = require('fs-extra');
const path = require('path');

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };

const GithubBinaryHost = 'https://prebuilt.libkungfu.io';

const defined = (e) => e;

const getScope = (npmConfigValue) =>
  npmConfigValue === 'undefined' ? '[package.json]' : '[user]';

const getPackageJson = (packageName) => {
  if (!packageName) {
    return fse.readJsonSync(path.resolve(process.cwd(), 'package.json'));
  }
  try {
    return fse.readJsonSync(require.resolve(`${packageName}/package.json`));
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

const isGithubEnv = () => process.env.CI && process.env.GITHUB_ACTIONS;

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

const findBinaryDependency = (packageJson) => {
  const hasBinary = (deps) =>
    Object.keys(deps)
      .map((key) => {
        const dependency = getPackageJson(key);
        if (dependency && dependency.binary) {
          return dependency.name;
        }
      })
      .filter(defined);
  return [packageJson.dependencies, packageJson.devDependencies]
    .filter(defined)
    .map(hasBinary)
    .flat();
};

const setBinaryHostConfig = (packageName) => {
  const packageJson = getPackageJson(packageName);
  if (!packageJson) {
    return;
  }
  if (packageJson.binary) {
    const key = `${packageJson.binary.module_name}_binary_host_mirror`;
    const binaryGithub = packageJson.binaryGithub;
    const override = binaryGithub && binaryGithub.host;
    const value = override ? binaryGithub.host : GithubBinaryHost;
    npmCall(['config', 'set', key, value]);
  }
};

const setAutoConfig = (key, globalScope = false) => {
  if (!isGithubEnv()) {
    return;
  }
  const packageJson = getPackageJson();
  if (packageJson.configGithub) {
    Object.keys(packageJson.configGithub).forEach((key) => {
      npmCall([
        'config',
        'set',
        `${packageJson.name}:${key}`,
        packageJson.configGithub[key],
      ]);
    });
  }
  if (packageJson.binary) {
    setBinaryHostConfig();
  }
  findBinaryDependency(packageJson).map(setBinaryHostConfig);
};

const showProjectConfig = (key) => {
  const packageJson = getPackageJson();
  const projectName = packageJson.name;
  const npmConfigKey = `${projectName}:${key}`;
  const npmConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    npmConfigValue === 'undefined' ? packageJson.config[key] : npmConfigValue;
  console.log(`[config] ${npmConfigKey} = ${value} ${getScope(npmConfigValue)}`);
};

const showBinaryHostConfig = (packageName) => {
  const packageJson = getPackageJson(packageName);
  if (!packageJson) {
    return;
  }
  const key = packageJson.binary.module_name;
  const npmConfigKey = `${key}_binary_host_mirror`;
  const hostConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    hostConfigValue === 'undefined' && packageJson
      ? packageJson.binary.host
      : hostConfigValue;
  console.log(`[binary] ${npmConfigKey} = ${value} ${getScope(hostConfigValue)}`);
};

const showAutoConfig = () => {
  const packageJson = getPackageJson();
  if (packageJson.config) {
    Object.keys(packageJson.config).map(showProjectConfig);
  }
  if (packageJson.binary) {
    showBinaryHostConfig();
  }
  findBinaryDependency(packageJson).map(showBinaryHostConfig);
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
  showAutoConfig: showAutoConfig,
};
