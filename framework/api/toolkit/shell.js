const { shell } = require('@kungfu-trader/kungfu-core');
const { say } = require('cfonts');
const chalk = require('chalk');
const fse = require('fs-extra');

module.exports = {
  ...shell,

  greeting: function () {
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
  },

  logStats: function (proc, data) {
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
  },

  runDist: function (distDir, run) {
    fse.ensureDirSync(distDir);
    run(distDir).catch(shell.utils.exitOnError);
  },
};
