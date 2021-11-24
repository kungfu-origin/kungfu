'use strict';

const chalk = require('chalk');
const electron = require('electron');
const fse = require('fs-extra');
const path = require('path');
const { say } = require('cfonts');
const { spawn } = require('child_process');
const webpack = require('webpack');

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
  } else console.log(chalk.yellow.bold('\n  kungfu'));
  console.log(chalk.blue('  getting ready...') + '\n');
}

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

function startMain(argv) {
  const mainConfig = require('./webpack.main.config')(argv);
//   const rootDir = path.dirname(path.dirname(__dirname));
//   const indexJs = path.resolve(rootDir, 'src', 'main', 'index.dev.js');
//   mainConfig.entry.main = [indexJs].concat(mainConfig.entry.main);

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
        process.kill(electronProcess.pid, "SIGINT");
        electronProcess = null;
        startElectron();
        setTimeout(() => {
          manualRestart = false;
        }, 5000);
      }
    });

    compiler.hooks.done.tap("main-compile-done", stats => {
      resolve();
    })
  });
}

function startElectron(argv) {
  const electronLog = (data, color) => {
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
  };

  electronProcess = spawn(
    electron,
    [
      '--inspect=5858',
      "--trace-warnings",
      "--trace-deprecation",
      "-enable-logging",
      path.join(argv.distDir, argv.distName, 'main.js')
    ],
    {
      cwd: path.dirname(argv.distDir)
    }
  );

  electronProcess.stdout.on('data', (data) => {
    electronLog(data, 'blue');
  });
  electronProcess.stderr.on('data', (data) => {
    electronLog(data, 'red');
  });
  electronProcess.on('close', () => {
    electronLog("exit", 'blue');
    if (!manualRestart) process.exit(0);
  });
}

const run = (distDir, distName = '') => {
  const argv = {
    mode: "development",
    distDir: distDir,
    distName: distName
  };

  const tasks = [startMain];
  const rootDir = path.dirname(path.dirname(__dirname));
  const coreDir = require.resolve('@kungfu-trader/kungfu-core');

  fse.rmSync(distDir, { recursive: true });
  process.chdir(rootDir);
  process.env.KFC_DIR = path.resolve(coreDir, '..', '..', 'dist', 'kfc');

  greeting();

  return Promise.all(tasks.map(f => f(argv))).then(() => startElectron(argv));
}

module.exports = run;

if (require.main === module) {
    run(path.resolve('./dist')).catch(err => console.log(err));
//   run(require('@kungfu-trader/kungfu-app2').defaultDistDir).catch(console.error);
}
