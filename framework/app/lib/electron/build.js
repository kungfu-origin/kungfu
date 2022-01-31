const path = require('path');

const run = (distDir) => {
  const electronBuilder = require('electron-builder');
  const baseConfig = require('./config');
  const pkgDir = path.dirname(distDir);
  process.chdir(pkgDir);

  console.log(`pwd: ${pkgDir}`);

  baseConfig.artifactName =
    '${productName}-${buildVersion}-${os}-${arch}-${channel}.${ext}';

  return electronBuilder.build({
    config: baseConfig,
  });
};

module.exports = run;

if (require.main === module) {
  run(require('@kungfu-trader/kungfu-app').defaultDistDir).catch(console.error);
}
