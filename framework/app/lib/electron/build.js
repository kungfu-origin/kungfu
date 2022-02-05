const run = (distDir) => {
  const electronBuilder = require('electron-builder');
  const path = require('path');
  const baseConfig = require('./config');
  const appConfig = {
    productName: 'Kungfu',
    artifactName:
      '${productName}-${buildVersion}-${os}-${arch}-${channel}.${ext}',
    directories: {
      output: path.join(
        'build',
        'stage',
        'kungfu',
        `v${version.major}`,
        `v${version}`,
      ),
    },
  };
  process.chdir(path.dirname(distDir));
  return electronBuilder.build({
    config: { ...baseConfig, ...appConfig },
  });
};

module.exports = run;

if (require.main === module) {
  run(require('@kungfu-trader/kungfu-app').defaultDistDir).catch(console.error);
}
