'use strict';

const chalk = require('chalk');
const electron = require('electron');
const fse = require('fs-extra');
const path = require('path');
const { say } = require('cfonts');
const { spawn } = require('child_process');
const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');
const {
  getAppDefaultDistDir,
  getAppDir,
  getKfcDir,
  getExtensionDirs,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');
const defaultDistDir = getAppDefaultDistDir();

let electronProcess = null;
let manualRestart = false;

function greeting() {
  const cols = process.stdout.columns;
  let text = '';

  if (cols > 104) text = '';
  else if (cols > 76) text = 'kungfu';
  else text = false;

  if (text) {
    say(text, {
      colors: ['yellow'],
      font: 'simple3d',
      space: false,
    });
  } else console.log(chalk.yellow.bold('\n  kungfu-dev'));
  console.log(chalk.blue('  getting ready...') + '\n');
}

function logStats(proc, data) {
  let log = '';
  log += chalk.yellow.bold(
    `┏ ${proc} Process ${new Array(19 - proc.length + 1).join('-')}`,
  );
  log += '\n\n';
  if (typeof data === 'object') {
    data
      .toString({
        colors: true,
        chunks: false,
      })
      .split(/\r?\n/)
      .forEach((line) => {
        log += '  ' + line + '\n';
      });
  } else {
    log += `  ${data}\n`;
  }
  log += '\n' + chalk.yellow.bold(`┗ ${new Array(28 + 1).join('-')}`) + '\n';
  console.log(log);
}

const electronLog = (data, color, type = 'Kungfu') => {
  let log = '';
  data = data.toString().split(/\r?\n/);
  data.forEach((line) => {
    log += `  ${line}\n`;
  });
  if (/[0-9A-z]+/.test(log)) {
    console.log(
      chalk[color].bold(`┏ ${type} ---------------------`) +
        '\n\n' +
        log +
        chalk[color].bold('┗ ----------------------------') +
        '\n',
    );
  }
};

function startRenderer(argv) {
  const rendererConfig = require('../webpack/webpack.renderer.config')(argv);
  return new Promise((resolve, reject) => {
    const compiler = webpack(rendererConfig);
    const server = new WebpackDevServer(
      {
        static: path.join(__dirname, '../'),
        port: 9090,
        hot: true,
        headers: {
          'Access-Control-Allow-Origin': '*',
        },
      },
      compiler,
    );
    server.start();

    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      logStats('Renderer', stats);
    });

    compiler.hooks.done.tap('renderer-compile-done', (stats) => {
      logStats('Renderer', stats);
      resolve();
    });
  });
}

function startMain(argv) {
  const mainConfig = require('../webpack/webpack.main.config')(argv);
  const appDir = getAppDir();
  const indexJs = path.resolve(appDir, 'src', 'main', 'index.dev.ts');
  mainConfig.entry.main = [indexJs].concat(mainConfig.entry.main);

  return new Promise((resolve, reject) => {
    const compiler = webpack(mainConfig);
    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      logStats('Main', stats);
      if (electronProcess && electronProcess.kill) {
        manualRestart = true;
        process.kill(electronProcess.pid, 'SIGINT');
        electronProcess = null;
        startElectron({
          mode: 'development',
          distDir: argv.distDir,
          distName: argv.distName,
        });
        setTimeout(() => {
          manualRestart = false;
        }, 20000);
      }
    });

    compiler.hooks.done.tap('main-compile-done', (stats) => {
      logStats('Main', stats);
      resolve();
    });
  });
}

function copyWebpackDist(argv) {
  const srcDir = path.join(getAppDir(), 'dist', 'app');
  const targetDir = path.join(argv.distDir, argv.distName);
  logStats('Builder', `copy from ${srcDir} to ${targetDir}`);
  fse.removeSync(targetDir);
  fse.copySync(srcDir, targetDir, {});
}

function startElectron(argv) {
  electronProcess = spawn(
    electron,
    [
      '--inspect=5858',
      '--trace-warnings',
      '--trace-deprecation',
      // '--enable-logging', windows上会出现cmd弹窗
      path.join(argv.distDir, argv.distName, 'main.js'),
    ],
    {
      cwd: path.dirname(argv.distDir),
      detached: false,
    },
  );

  electronProcess.stdout.on('data', (data) => {
    electronLog(data, 'blue');
  });
  electronProcess.stderr.on('data', (data) => {
    electronLog(data, 'red');
  });
  electronProcess.on('close', () => {
    electronLog('exit', 'blue');
    if (!manualRestart) {
      process.exit(0);
    }
  });
}

const run = (distDir, distName = 'app', withWebpack) => {
  greeting();

  const appDir = getAppDir();
  if (withWebpack) {
    process.chdir(appDir);
  } else {
    process.chdir(process.cwd());
  }

  const kfcDir = getKfcDir();
  const extdirs = getExtensionDirs();

  process.env.KFC_DIR = kfcDir;
  process.env.KFC_DEV = true;
  process.env.EXTENSION_DIRS = [distDir, ...extdirs].join(path.delimiter);

  const argv = {
    mode: 'development',
    distDir: distDir,
    distName: distName,
  };

  const tasks = withWebpack ? [startRenderer, startMain] : [copyWebpackDist];

  return Promise.all(tasks.map((f) => f(argv))).then(() => startElectron(argv));
};

module.exports = run;

if (require.main === module) {
  fse.ensureDirSync(defaultDistDir);
  run(defaultDistDir).catch(console.error);
}
