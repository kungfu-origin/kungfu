const { run, hasTool } = require('./node-lib.js');

function main() {
  const argv = process.argv.slice(2);
  if (hasTool('black')) {
    run('black', argv);
  } else {
    run('pipenv', ['run', 'black'].concat(argv));
  }
}

if (require.main === module) main();
