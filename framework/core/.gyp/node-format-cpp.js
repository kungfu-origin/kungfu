const { run } = require('./node-lib.js');
const glob = require('glob');
const path = require('path');

const main = (module.exports = function (argv) {
  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));

  run('clang-format', ['--version']);

  argv.forEach((dir) =>
    glob
      .sync(path.join(`${cwd}/${dir}/**`, '*.@(h|hpp|hxx|cpp|c|cc|cxx)'))
      .forEach((p) => {
        run('clang-format', ['-style=file', '-i', p], false);
      }),
  );
});

if (require.main === module) main(process.argv.slice(2));
