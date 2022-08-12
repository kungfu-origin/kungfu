const fse = require('fs-extra');
const path = require('path');

module.exports = {
  flags: 'clean',
  desc: 'Clean kungfu craft',
  run: () => {
    fse.removeSync(path.join(process.cwd(), 'build'));
    fse.removeSync(path.join(process.cwd(), 'dist'));
  },
};
