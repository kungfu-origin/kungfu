const run = (distDir) => {
  const electronBuilder = require('electron-builder');
  const path = require('path');
  const semver = require('semver');
  const baseConfig = require('./config');
  const cwd = path.dirname(distDir);
  const packageJson = require(`${path.join(cwd, 'package.json')}`);
  const version = semver.parse(packageJson.version);
  const craftName = packageJson.name.replace(/@.*\//g, '');
  const craftConfig = packageJson.kungfuCraft || { productName: 'Kungfu' };
  const appConfig = {
    artifactName:
      '${productName}-${buildVersion}-${os}-${arch}-${channel}.${ext}',
    directories: {
      output: path.join(
        'build',
        'stage',
        craftName,
        `v${version.major}`,
        `v${version}`,
      ),
    },
  };
  process.chdir(cwd);
  return electronBuilder.build({
    config: { ...baseConfig, ...craftConfig, ...appConfig },
  });
};

module.exports = run;

if (require.main === module) {
  run(require('@kungfu-trader/kungfu-app').defaultDistDir).catch(console.error);
}
