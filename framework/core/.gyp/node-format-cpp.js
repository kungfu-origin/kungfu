const { run, hasTool } = require('./node-lib.js');
const glob = require('glob');
const path = require('path');

function main() {
  if (hasTool('clang-format')) {
    glob
      .sync(path.join('src', '**', '*.@(h||hpp|hxx|cpp|c|cc|cxx)'))
      .forEach((p) => {
        run('clang-format', ['--verbose', '-style=file', '-i', p], true, true);
      });
  } else {
    run('pipenv', ['run', 'conan', 'source', '.']);
  }
}

if (require.main === module) main();
