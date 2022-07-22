const { spawnSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };

const GithubBinaryHost = 'https://prebuilt.libkungfu.io';

const defined = (e) => e;

const getScope = (npmConfigValue) => {
  return npmConfigValue === 'undefined' ? '[package.json]' : '[user]';
};

const getElectronArch = () => {
  try {
    const electron = require('electron');
    const electronVersionScript = path.resolve(
      path.dirname(__dirname),
      '.gyp',
      'electron-version.js',
    );
    const isLinux = process.platform === 'linux';
    const electronArgs = isLinux ? ['--no-sandbox'] : [];
    const result = runAndCollect(
      electron,
      [...electronArgs, electronVersionScript],
      {
        silent: true,
      },
    ).out;
    const resolveHeadless = (arch) => {
      return arch || getTargetArch();
    };
    return isLinux ? resolveHeadless(result) : result;
  } catch (e) {
    return undefined;
  }
};

const getTargetArch = () => {
  return getPackageJson('@kungfu-trader/kungfu-core').config.arch;
};

const getPackageJson = (packageName) => {
  const toJSON = (filepath) => {
    return JSON.parse(fs.readFileSync(filepath, 'utf8').toString());
  };
  if (!packageName) {
    return toJSON(path.resolve(process.cwd(), 'package.json'));
  }
  try {
    return toJSON(require.resolve(`${packageName}/package.json`));
  } catch (e) {
    return undefined;
  }
};

const getConfigValue = (name) => {
  return process.env[`npm_package_config_${name}`];
};

const getNpmConfigValue = (key) => {
  return runAndCollect('npm', ['config', 'get', key], { silent: true }).out;
};

const exitOnError = (error) => {
  console.error(error);
  process.exit(-1);
};

const trace = (cmd, argv, opts) => {
  if (!opts.silent) {
    console.log(`$ ${cmd} ${argv.join(' ')}`);
  }
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

const verifyElectron = () => {
  const electronArch = getElectronArch();
  const targetArch = getTargetArch();
  if (electronArch === targetArch) {
    return;
  }
  console.error(
    `Electron arch ${electronArch} does not match target ${targetArch}`,
  );
  console.log(`Try to fix it by reinstall electron [${targetArch}]`);
  try {
    const electronModulePath = path.dirname(
      require.resolve('electron/package.json'),
    );
    const yarnIntegrityPath = path.join(
      path.dirname(electronModulePath),
      '.yarn-integrity',
    );
    fs.rmSync(electronModulePath, {
      force: true,
      recursive: true,
    });
    fs.rmSync(yarnIntegrityPath, { force: true });
  } catch (e) {
    console.warn('Failed to remove electron module');
  }
  run(
    'yarn',
    ['install', '--frozen-lockfile', '--network-timeout=1000000', '--silent'],
    true,
    {
      silent: true,
      env: {
        ...process.env,
        npm_config_arch: targetArch,
      },
    },
  );
  const reinstalledElectronArch = getElectronArch();
  console.log(`Reinstall electron [${reinstalledElectronArch}] done`);
  if (reinstalledElectronArch !== targetArch) {
    console.error(
      `Reinstall electron [${reinstalledElectronArch}] failed to match [${targetArch}]`,
    );
    console.error(
      `Please fix it manually by yarn install with environment variable npm_config_arch set to ${targetArch}`,
    );
    process.exit(-1);
  }
};

const run = (cmd, argv = [], check = true, opts = {}) => {
  const real_cwd = fs.realpathSync(path.resolve(process.cwd()));
  trace(cmd, argv, opts);
  const result = spawnSync(cmd, argv, {
    shell: true,
    stdio: 'inherit',
    windowsHide: true,
    cwd: real_cwd,
    ...opts,
  });
  if (check && result.status !== 0) {
    process.exit(opts.tolerant ? 0 : result.status);
  }
  return result;
};

const runAndCollect = (cmd, argv = [], opts = {}) => {
  trace(cmd, argv, opts);
  const result = spawnSync(cmd, argv, {
    shell: true,
    stdio: 'pipe',
    windowsHide: true,
    ...opts,
  });
  result.out = result.stdout.toString().trim();
  return result;
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
  return [
    packageJson.dependencies || {},
    packageJson.optionalDependencies || {},
    packageJson.devDependencies || {},
  ]
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

const setAutoConfig = () => {
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
  console.log(
    `[config] ${npmConfigKey} = ${value} ${getScope(npmConfigValue)}`,
  );
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
  console.log(
    `[binary] ${npmConfigKey} = ${value} ${getScope(hostConfigValue)}`,
  );
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
  getElectronArch: getElectronArch,
  getTargetArch: getTargetArch,
  getPackageJson: getPackageJson,
  exitOnError: exitOnError,
  getConfigValue: getConfigValue,
  npmCall: npmCall,
  verifyElectron: verifyElectron,
  run: run,
  runAndCollect: runAndCollect,
  runAndExit: runAndExit,
  setAutoConfig: setAutoConfig,
  showAutoConfig: showAutoConfig,
};
