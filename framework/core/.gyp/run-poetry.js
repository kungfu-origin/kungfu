const fs = require('fs');
const path = require('path');
const { shell } = require('../lib');

function main(args) {
  if (fs.existsSync(path.join(process.cwd(), 'Pipfile.lock'))) {
    shell.runAndExit(
      'pipenv',
      ['run', 'python', '-m', 'poetry', ...args]
    );
  } else {
    shell.run('poetry', [...args], false);
  }
}

module.exports.main = main;

if (require.main === module) main(process.argv.slice(2));
