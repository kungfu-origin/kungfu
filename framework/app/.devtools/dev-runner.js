'use strict';

const chalk = require('chalk');
const electron = require('electron');
const path = require('path');
const { say } = require('cfonts');
const { spawn } = require('child_process');
const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');
const webpackHotMiddleware = require('webpack-hot-middleware')

const mainConfig = require('../.webpack/webpack.main.config');
const rendererConfig = require('../.webpack/webpack.renderer.config');
const daemonConfig = require('../.webpack/webpack.daemon.config');

let electronProcess = null;
let manualRestart = false;
var hotMiddleware;

function logStats(proc, data) {
  let log = '';

  log += chalk.yellow.bold(`┏ ${proc} Process ${new Array(19 - proc.length + 1).join('-')}`);
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

function startRenderer() {
  return new Promise((resolve, reject) => {

    const compiler = webpack(rendererConfig);
    hotMiddleware = webpackHotMiddleware(compiler, {
      log: true,
      heartbeat: 2500
    })

    compiler.hooks.afterEmit.tap('afterEmit', () => {
      console.log("compiler.hooks afterEmit", '=====')
      hotMiddleware.publish({
        action: 'reload'
      })
    })

    compiler.hooks.done.tap('done', stats => {
      console.log("compiler.hooks done", '=====')
      logStats('Renderer', stats)
    })

    const opts = {
      port: 9090,
      static: path.join(__dirname, '../'),
    };

    const server = new WebpackDevServer(opts, compiler);
    server.start();
    resolve();
  });
}

function startMain() {
  return new Promise((resolve, reject) => {
    mainConfig.entry.main = [path.join(path.dirname(__dirname), 'src', 'main', 'index.dev.js')].concat(
      mainConfig.entry.main,
    );

    const compiler = webpack(mainConfig);

    compiler.hooks.watchRun.tapAsync('watch-run', (compilation, done) => {
      logStats('Main', chalk.white.bold('compiling...'))
      hotMiddleware && hotMiddleware.publish({ action: 'compiling' })
      done()
    })

    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      logStats('Main', stats);

      if (electronProcess && electronProcess.kill) {
        manualRestart = true;
        process.kill(electronProcess.pid);
        electronProcess = null;
        startElectron();

        setTimeout(() => {
          manualRestart = false;
        }, 5000);
      }

      resolve();
    });
  });
}

function startDaemon() {
  return new Promise((resolve, reject) => {
    daemonConfig.entry.daemon = [].concat(daemonConfig.entry.daemon);

    const compiler = webpack(daemonConfig);

    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      logStats('Daemon', stats);
      resolve();
    });
  });
}

function startElectron() {
  electronProcess = spawn(electron, ['--inspect=5858', '--enable-logging', '--trace-warnings', '.']);

  electronProcess.stdout.on('data', (data) => {
    electronLog(data, 'blue');
  });
  electronProcess.stderr.on('data', (data) => {
    electronLog(data, 'red');
  });

  electronProcess.on('close', () => {
    if (!manualRestart) process.exit();
  });
}

function electronLog(data, color) {
  let log = '';
  data = data.toString().split(/\r?\n/);
  data.forEach((line) => {
    log += `  ${line}\n`;
  });
  if (/[0-9A-z]+/.test(log)) {
    console.log(
      chalk[color].bold('┏ KungFu ---------------------') +
        '\n\n' +
        log +
        chalk[color].bold('┗ ----------------------------') +
        '\n',
    );
  }
}

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
  } else console.log(chalk.yellow.bold('\n  kungfu'));
  console.log(chalk.blue('  getting ready...') + '\n');
}

function init() {
  greeting();

  Promise.all([startRenderer(), startMain(), startDaemon()])
    .then(() => {
      startElectron();
    })
    .catch((err) => {
      console.error(err);
    });
}

init();
