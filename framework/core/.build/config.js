#!/usr/bin/env node

const fs = require('fs-extra');
const path = require('path');
const { spawnSync } = require('child_process');

const PrebuiltHostConfigKey = 'kungfu_node_binary_host_mirror';

const PyPI_US = 'https://pypi.python.org/simple';
const PyPI_CN = 'https://mirrors.aliyun.com/pypi/simple';

const PrebuiltHost_CN = 'https://kungfu-prebuilt.s3.cn-northwest-1.amazonaws.com.cn';
const PrebuiltHost_US = 'https://kungfu-prebuilt.s3.us-east-1.amazonaws.com';

const spawnOptsPipe = { shell: true, stdio: 'pipe', windowsHide: true };
const spawnOptsInherit = { shell: true, stdio: 'inherit', windowsHide: true };

const packageJson = fs.readJsonSync(path.resolve(path.dirname(__dirname), 'package.json'));
const projectName = packageJson.name;

const scope = (npmConfigValue) => (npmConfigValue === 'undefined' ? '[package.json]' : '[user]');

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
  console.log(`${npmConfigKey} = ${value} ${scope(npmConfigValue)}`);
}

function showAllConfig() {
  Object.keys(packageJson.config).map(showProjectConfig);

  const hostConfigValue = getNpmConfigValue(PrebuiltHostConfigKey);
  const value = hostConfigValue === 'undefined' ? packageJson.binary.host : hostConfigValue;
  console.log(`${PrebuiltHostConfigKey} = ${value} ${scope(hostConfigValue)}`);
}

function npmCall(npmArgs) {
  console.log(`$ npm ${npmArgs.join(' ')}`);
  const result = spawnSync('npm', npmArgs, spawnOptsInherit);
  if (result.status !== 0) {
    console.error(`Failed with status ${status}`);
    process.exit(result.status);
  }
}

exports.argv = require('yargs/yargs')(process.argv.slice(2))
  .command(
    'auto',
    'Set npm configs automatically',
    () => {},
    () => {
      const githubActions = 'GITHUB_ACTIONS' in process.env;
      const pypi = githubActions ? PyPI_US : PyPI_CN;
      const prebuiltHost = githubActions ? PrebuiltHost_US : PrebuiltHost_CN;

      const setConfig = (key, value) => githubActions && npmCall(['config', 'set', key, value]);
      setConfig(`${projectName}:pypi`, pypi);
      setConfig(PrebuiltHostConfigKey, prebuiltHost);

      showAllConfig();
    },
  )
  .command(
    'show',
    'Show npm configs',
    () => {},
    () => {
      showAllConfig();
    },
  )
  .command(
    'dir',
    'Show kungfu core base directory',
    () => {},
    () => {
      console.log(fs.realpathSync(path.dirname(__dirname)));
    },
  )
  .command(
    'info',
    'Show kungfu core build info',
    () => {},
    () => {
      const buildinfoPath = path.join(path.dirname(__dirname), 'build', 'kfc', 'kungfubuildinfo.json');
      if (fs.existsSync(buildinfoPath)) {
        const buildinfo = require(buildinfoPath);
        console.log(buildinfo);
      } else {
        console.warn(`Info file missing, ${buildinfoPath} not exists`);
      }
    },
  )
  .demandCommand()
  .help().argv;
