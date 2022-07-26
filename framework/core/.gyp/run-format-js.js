const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  const prettierConfig = path.resolve(
    path.dirname(__dirname),
    '.prettierrc.js',
  );
  const prettierArgs = ['-l', '-w', '--config', prettierConfig];
  shell.run('prettier', ['--version'], false);
  shell.runAndExit('prettier', prettierArgs.concat(argv));
  shell.run('yarn', ['run', 'prettier'].concat(prettierArgs).concat(argv));
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2).map((a) => `"${a}"`));
}