'use strict';

const { say } = require('cfonts');
const chalk = require('chalk');
const fse = require('fs-extra');
const path = require('path');
const webpack = require('webpack');
const Multispinner = require('multispinner');
const {
  getAppDefaultDistDir,
  getAppDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

function greeting() {
  const isCI = process.env.CI || false;
  const cols = process.stdout.columns;
  let text = '';

  if (cols > 85) text = 'kungfu-build';
  else if (cols > 60) text = 'kungfu-|build';
  else text = false;

  if (text && !isCI) {
    say(text, {
      colors: ['yellow'],
      font: 'simple3d',
      space: false,
    });
  } else console.log(chalk.yellow.bold('\n  kungfu-build'));
}

const pack = (config) =>
  new Promise((resolve, reject) => {
    webpack(config, (err, stats) => {
      if (err) {
        reject(err.stack || err);
      } else if (stats.hasErrors()) {
        let err = '';

        stats
          .toString({
            chunks: false,
            colors: true,
          })
          .split(/\r?\n/)
          .forEach((line) => {
            err += `    ${line}\n`;
          });

        reject(err);
      } else {
        resolve(
          stats.toString({
            chunks: false,
            colors: true,
          }),
        );
      }
    });
  });

const run = (distDir, distName = 'app') => {
  fse.rmSync(distDir, { recursive: true });
  const appDir = getAppDir();
  process.chdir(appDir);
  greeting();

  return new Promise((resolve, reject) => {
    const argv = {
      mode: 'production',
      distDir: distDir,
      distName: distName,
    };

    const mainConfig = require('./webpack.main.config');
    const rendererConfig = require('./webpack.renderer.config');

    const errorLog = chalk.bgRed.white(' ERROR ') + ' ';
    const okayLog = chalk.bgBlue.white(' OKAY ') + ' ';

    const tasks = ['main', 'renderer'];
    const spinner = new Multispinner(tasks, {
      preText: 'building',
      postText: 'process',
      interval: process.env.GITHUB_ACTIONS ? 60000 : 100,
    });

    let results = '';

    spinner.on('success', () => {
      const config = require('@kungfu-trader/kungfu-app/package.json');
      fse.copySync(
        require.resolve('@kungfu-trader/kungfu-core/dist/kfc/drone.node'),
        path.join(distDir, distName, `${config.binary.module_name}.node`),
        {},
      );
      process.stdout.write('\x1B[2J\x1B[0f');
      console.log(`\n\n${results}`);
      console.log(`${okayLog}webpack ${chalk.yellow('`done`')}\n`);
      resolve();
    });

    spinner.on('error', reject);

    pack(mainConfig(argv))
      .then((result) => {
        results += result + '\n\n';
        spinner.success('main');
      })
      .catch((err) => {
        spinner.error('main');
        console.log(`\n  ${errorLog}failed to build main process`);
        console.error(`\n${err}\n`);
        process.exit(1);
      });

    pack(rendererConfig(argv))
      .then((result) => {
        results += result + '\n\n';
        spinner.success('renderer');
      })
      .catch((err) => {
        spinner.error('renderer');
        console.log(`\n  ${errorLog}failed to build renderer process`);
        console.error(`\n${err}\n`);
        process.exit(1);
      });
  });
};

module.exports = run;

if (require.main === module) {
  const defaultDistDir = getAppDefaultDistDir();
  fse.ensureDirSync(defaultDistDir);
  run(defaultDistDir).catch(console.error);
}
