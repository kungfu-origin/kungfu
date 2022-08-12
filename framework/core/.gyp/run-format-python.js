const fs = require('fs');
const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  const cwd = process.cwd().toString();
  const coreDir = path.dirname(__dirname);

  const test = shell.run('black', ['--version'], false);
  if (test.status === 0) {
    shell.runAndExit('black', argv);
  }

  process.chdir(coreDir);
  const pipenvPath = path.resolve(
    shell.runAndCollect('pipenv', ['--py'], { silent: true }).out,
  );
  const blackBin = process.platform === 'win32' ? 'black.exe' : 'black';
  const blackPath = path.resolve(path.dirname(pipenvPath), blackBin);

  if (fs.existsSync(blackPath)) {
    process.chdir(cwd);
    shell.run(blackPath, ['--version'], false);
    shell.run(blackPath, argv, true, { tolerant: true });
  }
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2));
}
