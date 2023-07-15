const run = (distDir) => {
  const electronBuilder = require('electron-builder');
  const path = require('path');
  const semver = require('semver');
  const baseConfig = require('./config');
  const cwd = path.dirname(distDir);
  const packageJson = require(`${path.join(cwd, 'package.json')}`);
  const version = semver.parse(packageJson.version);
  const craftName = packageJson.name.replace(/@.*\//g, '');
  const craftConfig = {
    appId: packageJson?.kungfuCraft?.appId || 'Kungfu.Origin.KungFu.Trader',
    productName: packageJson?.kungfuCraft?.productName || 'Kungfu',
  };
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
  const notarizeConfig = process.env.CSC_LINK
    ? {
        afterSign: 'electron-builder-notarize',
        mac: {
          ...baseConfig.mac,
          hardenedRuntime: true,
          entitlements:
            './node_modules/electron-builder-notarize/entitlements.mac.inherit.plist',
        },
      }
    : {};
  process.chdir(cwd);
  return electronBuilder.build({
    config: { ...baseConfig, ...craftConfig, ...appConfig, ...notarizeConfig },
  });
};

module.exports = run;

if (require.main === module) {
  const app = require('@kungfu-trader/kungfu-app');
  run(app.defaultDistDir).catch(console.error);
}
