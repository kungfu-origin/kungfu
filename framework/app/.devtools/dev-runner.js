'use strict';

const chalk = require('chalk');
const electron = require('electron');
const path = require('path');
const { say } = require('cfonts');
const { spawn } = require('child_process');
const webpack = require('webpack');
const WebpackDevServer = require('webpack-dev-server');

const mainConfig = require('../.webpack/webpack.main.config');
const rendererConfig = require('../.webpack/webpack.renderer.config');
const daemonConfig = require('../.webpack/webpack.daemon.config');

let electronProcess = null;
let manualRestart = false;

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

    // compiler.plugin('compilation', compilation => {
    //   compilation.plugin('html-webpack-plugin-after-emit', (data, cb) => {
    //     hotMiddleware.publish({ action: 'reload' })
    //     cb()
    //   })
    // })

    // compiler.plugin('done', stats => {
    //   logStats('Renderer', stats)
    // })

    const opts = {
      port: 9090,
      static: path.join(__dirname, '../'),
      headers: {
        'Access-Control-Allow-Origin': '*',
      },
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

    // compiler.plugin('watch-run', (compilation, done) => {
    //   logStats('Main', chalk.white.bold('compiling...'))
    //   hotMiddleware.publish({ action: 'compiling' })
    //   done()
    // })

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
  electronProcess = spawn(electron, ['--inspect=5858', '.']);

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