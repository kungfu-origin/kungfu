const { run } = require('./node-lib.js');
const glob = require('glob');
const path = require('path');

function main() {
  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));
  glob
    .sync(path.join(`${cwd}/src/**`, '*.@(h|hpp|hxx|cpp|c|cc|cxx)'))
    .forEach((p) => {
      run('clang-format', ['-style=file', '-i', p], false);
    });
}

if (require.main === module) main();
