'use strict';

const chalk = require('chalk');
const fse = require('fs-extra');
const { say } = require('cfonts');
const webpack = require('webpack');
const path = require('path');
const {
  getKungfuConfigKey,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

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

function startUi(argv) {
  const uiConfig = require('./webpack.config')(argv);
  return new Promise((resolve, _) => {
    const compiler = webpack(uiConfig);
    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err);
        return;
      }

      logStats('Ui', stats);
    });

    compiler.hooks.done.tap('ui-compile-done', (stats) => {
      logStats('Ui', stats);
      resolve();
    });
  });
}

const run = (distDir, distName = 'app') => {
  greeting();
  const argv = {
    mode: 'development',
    distDir: distDir,
    distName: distName,
  };

  return Promise.all([startUi(argv)]);
};

module.exports = run;

if (require.main === module) {
  const distDir = path.join(process.cwd(), 'dist');
  const distName = getKungfuConfigKey();
  fse.ensureDirSync(distDir);
  run(distDir, distName).catch(console.error);
}
