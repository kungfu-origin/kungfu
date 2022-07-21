const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  shell.run('black', ['--version'], false);

  shell.runAndExit('black', argv);

  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));
  const pipenvPath = path.resolve(
    shell.runAndCollect('pipenv', ['--py']).stdout.toString().trim(),
  );
  const blackBin = process.platform === 'win32' ? 'black.exe' : 'black';
  const blackPath = path.resolve(path.dirname(pipenvPath), blackBin);
  process.chdir(cwd);
  shell.runAndExit(blackPath, argv);

  process.chdir(path.dirname(__dirname));
  shell.run('pipenv', ['run', 'black'].concat(argv));
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2));
}
