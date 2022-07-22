const libnode = require('@kungfu-trader/libnode');
const fs = require('fs');
const fse = require('fs-extra');
const glob = require('glob');
const path = require('path');
const { shell } = require('../lib');

function main() {
  const buildType = shell.getConfigValue('build_type');
  glob.sync('*.*', { cwd: libnode.libpath }).forEach((p) => {
    const src = path.join(libnode.libpath, p);
    const dst = path.join('build', buildType, path.basename(p));
    if (fs.lstatSync(src).isFile()) {
      console.log(`$ cp ${src} ${dst}`);
      fse.copySync(src, dst, { overwrite: true });
    }
  });
}

module.exports.main = main;

if (require.main === module) main();
