const { getConfigValue } = require('./node-lib.js');
const libnode = require('@kungfu-trader/libnode');
const fs = require('fs');
const fse = require('fs-extra');
const glob = require('glob');
const path = require('path');

async function main() {
  const buildType = getConfigValue('build_type');
  glob.sync(path.join(libnode.libpath, '*.*')).forEach((p) => {
    if (fs.lstatSync(p).isFile()) {
      fse.copySync(p, path.join('build', buildType, path.basename(p)));
    }
  });
}

if (require.main === module) main();
