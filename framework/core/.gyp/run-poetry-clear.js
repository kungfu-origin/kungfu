const fs = require('fs');
const path = require('path');
const { shell } = require('../lib');

function main() {
  const poetryArgs = ['cache', 'clear', '--all', 'pypi'];
  if (fs.existsSync(path.join(process.cwd(), 'Pipfile.lock'))) {
    shell.run(
      'pipenv',
      ['run', 'python', '-m', 'poetry', ...poetryArgs],
      false,
    );
  }
  shell.run('poetry', [...poetryArgs], false);
}

module.exports.main = main;

if (require.main === module) main();
