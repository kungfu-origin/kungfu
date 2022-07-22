const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  shell.run('poetry', ['--version'], false);
  shell.run('poetry', ['lock', '-n', '-q', '--no-update'], false);

  shell.run('black', ['--version'], false);
  shell.runAndExit('black', argv);

  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));
  const pipenvPath = path.resolve(shell.runAndCollect('pipenv', ['--py']).out);
  const blackBin = process.platform === 'win32' ? 'black.exe' : 'black';
  const blackPath = path.resolve(path.dirname(pipenvPath), blackBin);
  process.chdir(cwd);
  shell.runAndExit(blackPath, argv);

  process.chdir(path.dirname(__dirname));
  shell.run('pipenv', ['run', 'black', '--version'], true, { tolerant: true });
  shell.run('pipenv', ['run', 'black'].concat(argv));
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2));
}
