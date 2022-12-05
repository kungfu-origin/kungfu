// SPDX-License-Identifier: Apache-2.0

const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  const prettierConfig = path.resolve(
    path.dirname(__dirname),
    '.prettierrc.js',
  );
  const prettierArgs = ['-l', '-w', '--config', prettierConfig];
  const test = shell.run('prettier', ['--version'], false);
  if (test.status === 0) {
    shell.runAndExit('prettier', prettierArgs.concat(argv));
  }
  shell.run('yarn', ['run', 'prettier'].concat(prettierArgs).concat(argv));
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2).map((a) => `"${a}"`));
}
