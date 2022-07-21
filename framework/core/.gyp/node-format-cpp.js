const glob = require('glob');
const path = require('path');
const { shell } = require('../lib');

function main(argv) {
  const cwd = process.cwd();
  process.chdir(path.dirname(__dirname));

  shell.run('clang-format', ['--version'], true, { tolerant: true });

  argv.forEach((dir) =>
    glob
      .sync('**/*.@(h|hpp|hxx|cpp|c|cc|cxx)', { cwd: path.join(cwd, dir) })
      .forEach((p) => {
        const file = path.join(cwd, dir, p);
        shell.run('clang-format', ['-style=file', '-i', file]);
      }),
  );
}

module.exports.main = main;

if (require.main === module) {
  main(process.argv.slice(2));
}
