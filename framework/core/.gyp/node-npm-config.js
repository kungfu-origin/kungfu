#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const { spawnSync } = require('child_process');

const PyPI_US = 'https://pypi.python.org/simple';
const PyPI_CN = 'https://mirrors.aliyun.com/pypi/simple';

const PrebuiltModules = { '@kungfu-trader/kungfu-core': 'kungfu_node', '@kungfu-trader/libnode': 'link_node' };
const PrebuiltHostConfig = 'binary_host_mirror';
const PrebuiltHost_CN = 'https://prebuilt.libkungfu.cc';
const PrebuiltHost_US = 'https://prebuilt.libkungfu.io';

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };

const packageJson = fs.readJsonSync(path.resolve(path.dirname(__dirname), 'package.json'));
const projectName = packageJson.name;

const scope = (npmConfigValue) => (npmConfigValue === 'undefined' ? '[package.json]' : '[user]');

function getPackageJson(module) {
  try {
    return fs.readJsonSync(require.resolve(`${module}/package.json`));
  } catch (e) {
    return undefined;
  }
}

function getNpmConfigValue(key) {
  return spawnSync('npm', ['config', 'get', key], spawnOptsPipe)
    .output.filter((e) => e && e.length > 0)
    .toString()
    .trimEnd();
}

function showProjectConfig(key) {
  const npmConfigKey = `${projectName}:${key}`;
  const npmConfigValue = getNpmConfigValue(npmConfigKey);
  const value = npmConfigValue === 'undefined' ? packageJson.config[key] : npmConfigValue;
  console.log(`[config] ${npmConfigKey} = ${value} ${scope(npmConfigValue)}`);
}

function showPrebuiltHostConfig(module) {
  const packageJson = getPackageJson(module);
  const key = packageJson ? packageJson.binary.module_name : PrebuiltModules[module];
  const npmConfigKey = `${key}_${PrebuiltHostConfig}`;
  const hostConfigValue = getNpmConfigValue(npmConfigKey);
  const value = hostConfigValue === 'undefined' && packageJson ? packageJson.binary.host : hostConfigValue;
  console.log(`[binary] ${npmConfigKey} = ${value} ${scope(hostConfigValue)}`);
}

function showAllConfig() {
  Object.keys(packageJson.config).map(showProjectConfig);
  Object.keys(PrebuiltModules).map(showPrebuiltHostConfig);
}

function npmCall(npmArgs) {
  console.log(`$ npm ${npmArgs.join(' ')}`);
  const result = spawnSync('npm', npmArgs, spawnOptsInherit);
  if (result.status !== 0) {
    console.error(`Failed with status ${status}`);
    process.exit(result.status);
  }
}

require('@kungfu-trader/kungfu-core').sywac(module, (cli) => {
  cli.command('show', { run: showAllConfig }).command('auto', {
    run: () => {
      const githubActions = 'GITHUB_ACTIONS' in process.env;
      const pypi = githubActions ? PyPI_US : PyPI_CN;
      const prebuiltHost = githubActions ? PrebuiltHost_US : PrebuiltHost_CN;
      const setConfig = (key, value) => githubActions && npmCall(['config', 'set', key, value]);
      const setPrebuiltHost = (module) => setConfig(PrebuiltModules[module], prebuiltHost);

      setConfig(`${projectName}:pypi_mirror`, pypi);
      Object.keys(PrebuiltModules).map(setPrebuiltHost);

      showAllConfig();
    },
  });
});
