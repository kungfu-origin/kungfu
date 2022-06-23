const { runAndCollect, runAndExit, run } = require('./node-lib.js');
const path = require('path');

const main = (module.exports = function (argv) {
  const prettierConfig = path.resolve(
    path.dirname(__dirname),
    '.prettierrc.js',
  );
  const prettierArgs = ['-l', '-w', '--config', prettierConfig];
  runAndExit('prettier', prettierArgs.concat(argv));
  run('yarn', ['run', 'prettier'].concat(prettierArgs).concat(argv));
});

if (require.main === module) main(process.argv.slice(2).map((a) => `"${a}"`));
