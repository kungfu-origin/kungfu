const { runAndCollect, runAndExit, run } = require('./node-lib.js');
const path = require('path');

const main = (module.exports = function (argv) {
  run('black', ['--version'], false);

  runAndExit('black', argv);

  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));
  const pipenvPath = path.resolve(
    runAndCollect('pipenv', ['--py']).stdout.toString().trim(),
  );
  const blackBin = process.platform === 'win32' ? 'black.exe' : 'black';
  const blackPath = path.resolve(path.dirname(pipenvPath), blackBin);
  process.chdir(cwd);
  runAndExit(blackPath, argv);

  process.chdir(path.dirname(__dirname));
  run('pipenv', ['run', 'black'].concat(argv));
});

if (require.main === module) main(process.argv.slice(2));
