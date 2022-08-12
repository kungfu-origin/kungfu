const { shell } = require('../lib');

function main() {
  const tryFormat = (lang) => {
    shell.run('yarn', ['-s', `format:${lang}`], false, { silent: true });
  };
  try {
    tryFormat('cpp');
    tryFormat('python');
    tryFormat('js');
  } catch (err) {
    console.error(err);
  }
}

module.exports.main = main;

if (require.main === module) main();
