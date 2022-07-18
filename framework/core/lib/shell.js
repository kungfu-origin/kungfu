const { spawnSync } = require('child_process');
const fs = require('fs-extra');
const path = require('path');

const PyPI_US = 'https://pypi.python.org/simple';
const PyPI_CN = 'https://mirrors.aliyun.com/pypi/simple';

const PrebuiltModules = {
  '@kungfu-trader/kungfu-core': 'kungfu_node',
  '@kungfu-trader/libnode': 'link_node',
};

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };


const scope = (npmConfigValue) =>
  npmConfigValue === 'undefined' ? '[package.json]' : '[user]';

const getPackageJson = (module) => {
  if (!module) {
    return fs.readJsonSync(
      path.resolve(path.dirname(__dirname), 'package.json'),
    );
  }
  try {
    return fs.readJsonSync(require.resolve(`${module}/package.json`));
  } catch (e) {
    return undefined;
  }
}

const getNpmConfigValue = (key) => {
  return spawnSync('npm', ['config', 'get', key], spawnOptsPipe)
    .output.filter((e) => e && e.length > 0)
    .toString()
    .trimEnd();
}

const showProjectConfig = (key) => {
  const npmConfigKey = `${projectName}:${key}`;
  const npmConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    npmConfigValue === 'undefined' ? packageJson.config[key] : npmConfigValue;
  console.log(`[config] ${npmConfigKey} = ${value} ${scope(npmConfigValue)}`);
}

const showPrebuiltHostConfig = (module) => {
  const packageJson = getPackageJson(module);
  const key = packageJson
    ? packageJson.binary.module_name
    : PrebuiltModules[module];
  const npmConfigKey = `${key}_${PrebuiltHostConfig}`;
  const hostConfigValue = getNpmConfigValue(npmConfigKey);
  const value =
    hostConfigValue === 'undefined' && packageJson
      ? packageJson.binary.host
      : hostConfigValue;
  console.log(`[binary] ${npmConfigKey} = ${value} ${scope(hostConfigValue)}`);
}

const showAllConfig = () => {
  Object.keys(packageJson.config).map(showProjectConfig);
  Object.keys(PrebuiltModules).map(showPrebuiltHostConfig);
}

const npmCall = (npmArgs) => {
  console.log(`$ npm ${npmArgs.join(' ')}`);
  const result = spawnSync('npm', npmArgs, spawnOptsInherit);
  if (result.status !== 0) {
    console.error(`Failed with status ${status}`);
    process.exit(result.status);
  }
}

const exitOnError = (error) => {
  console.error(error);
  process.exit(-1);
};

const getConfigValue = (name) => {
  return process.env[`npm_package_config_${name}`];
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

module.exports = {
  exitOnError: exitOnError,
  getConfigValue: getConfigValue,
  run: run,
  runAndCollect: runAndCollect,
  runAndExit: runAndExit,
}
