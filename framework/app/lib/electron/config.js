const path = require('path');
const fse = require('fs-extra');
const semver = require('semver');
const findWorkspaceRoot = require('find-yarn-workspace-root');
const {
  getAppDir,
  getKfcDir,
  getCoreDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const appDir = getAppDir();
const kfcDir = getKfcDir();
const coreDir = getCoreDir();
const coreJSON = fse.readJSONSync(path.join(coreDir, 'package.json'));
const version = semver.parse(coreJSON.version);

const packageJSON = fse.readJSONSync(
  path.resolve(process.cwd(), 'package.json'),
);

const extdirs = Object.keys(packageJSON.dependencies || {})
  .map((name) => {
    const jsonPath = require.resolve(name + '/package.json');
    const json = fse.readJSONSync(jsonPath);
    if (json.kungfuConfig) {
      return path.dirname(jsonPath);
    }

    return null;
  })
  .filter((fullpath) => !!fullpath);

const extras = extdirs.map((fullpath) => {
  return {
    from: path.resolve(fullpath, 'dist'),
    to: 'kungfu-extensions',
  };
});

module.exports = {
  productName: 'Kungfu',
  artifactName:
    '${productName}-${buildVersion}-${os}-${arch}-${channel}.${ext}',
  generateUpdatesFilesForAllChannels: true,
  appId: 'Kungfu.Origin.KungFu.Trader',
  electronVersion: '13.2.0',
  directories: {
    output: path.join(
      'build',
      'stage',
      'kungfu',
      `v${version.major}`,
      `v${version}`,
    ),
  },
  publish: [
    {
      provider: 'generic',
      url: 'http://taurusai.com/kungfu',
    },
  ],
  npmRebuild: false,
  files: [
    'dist/app/**/*',
    '!**/@kungfu-trader/kfx-*/**/*',
    '!**/@kungfu-trader/kungfu-core/*',
    '!**/@kungfu-trader/kungfu-core/**/*',
    '**/@kungfu-trader/kungfu-core/lib/*',
    '**/@kungfu-trader/kungfu-core/*.json',
    '!**/@kungfu-trader/kungfu-app/*',
    '!**/@kungfu-trader/kungfu-app/**/*',
    '!**/@kungfu-trader/kungfu-js-api/*',
    '!**/@kungfu-trader/kungfu-js-api/**/*',
  ],
  extraResources: [
    {
      from: kfcDir,
      to: 'kfc',
      filter: ['!**/btdata'],
    },
    {
      from: `${coreDir}/build/python/dist`,
      to: 'kungfu-resources/python',
      filter: ['*.whl'],
    },
    {
      from: appDir,
      to: 'app/dist/app',
      filter: ['public/*', 'public/logo'],
    },
    {
      from: appDir,
      to: 'app',
      filter: ['lib/*'],
    },
    {
      from: `${appDir}/public`,
      to: 'kungfu-resources',
      filter: ['config/*', 'key/*'],
    },
    {
      from: `${appDir}/lib/electron`,
      to: 'app',
      filter: ['package.json'],
    },
    ...extras,
  ],
  asar: false,
  dmg: {
    contents: [
      {
        x: 410,
        y: 150,
        type: 'link',
        path: '/Applications',
      },
      {
        x: 130,
        y: 150,
        type: 'file',
      },
    ],
  },
  mac: {
    icon: `${appDir}/public/logo/icon.icns`,
    type: 'distribution',
    target: ['dmg'],
  },
  win: {
    icon: `${appDir}/public/logo/icon.ico`,
    target: [
      {
        target: 'nsis',
        arch: ['x64'],
      },
    ],
  },
  linux: {
    icon: `${appDir}/public/logo/icon.icns`,
    target: ['rpm', 'appimage'],
  },
  nsis: {
    oneClick: false,
    allowElevation: true,
    allowToChangeInstallationDirectory: true,
    installerIcon: `${appDir}/public/logo/icon.ico`,
    uninstallerIcon: `${appDir}/public/logo/icon.ico`,
    installerHeaderIcon: `${appDir}/public/logo/icon.ico`,
    createDesktopShortcut: true,
    createStartMenuShortcut: true,
  },
};
