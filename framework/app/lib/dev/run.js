'use strict';

const chalk = require('chalk');
const electron = require('electron');
const fse = require('fs-extra');
const path = require('path');
const { spawn } = require('child_process');
const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');
const { shell, utils } = require('@kungfu-trader/kungfu-js-api/toolkit');

let electronProcess = null;
let manualRestart = false;

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

      shell.logStats('Renderer', stats);
    });

    compiler.hooks.done.tap('renderer-compile-done', (stats) => {
      shell.logStats('Renderer', stats);
      resolve();
    });
  });
}

function startMain(argv) {
  const mainConfig = require('../webpack/webpack.main.config')(argv);
  const appDir = utils.getAppDir();
  const indexJs = path.resolve(appDir, 'src', 'main', 'index.dev.ts');
  mainConfig.entry.main = indexJs;

  return new Promise((resolve, reject) => {
    const compiler = webpack(mainConfig);
    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      shell.logStats('Main', stats);
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
      shell.logStats('Main', stats);
      resolve();
    });
  });
}

function copyWebpackDist(argv) {
  const srcDir = path.join(utils.getAppDir(), 'dist', 'app');
  const publicDir = path.join(utils.getAppDir(), 'public');
  const targetDir = path.join(argv.distDir, argv.distName);
  const publicTargetDir = path.join(argv.distDir, 'public');

  shell.logStats('Builder', `copy from ${srcDir} to ${targetDir}`);
  fse.removeSync(targetDir);
  fse.copySync(srcDir, targetDir);

  shell.logStats('Builder', `copy from ${publicDir} to ${publicTargetDir}`);
  fse.removeSync(publicTargetDir);
  fse.copySync(publicDir, publicTargetDir);
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
  shell.greeting();

  const appDir = utils.getAppDir();
  const argv = utils.buildDevArgv(distDir, distName);
  const tasks = withWebpack ? [startRenderer, startMain] : [copyWebpackDist];

  if (withWebpack) {
    process.chdir(appDir);
  } else {
    process.chdir(process.cwd().toString());
  }

  return Promise.all(tasks.map((f) => f(argv))).then(() => startElectron(argv));
};

module.exports = run;

if (require.main === module) {
  shell.runDist(utils.getAppDefaultDistDir(), run);
}
