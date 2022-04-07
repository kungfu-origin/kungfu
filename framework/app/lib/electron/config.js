const path = require('path');
const kungfuCore = require('@kungfu-trader/kungfu-core/package.json');
const {
  getAppDir,
  getKfcDir,
  getCoreDir,
  getExtensionDirs,
  getCliDir,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const appDir = getAppDir();

const kfcDir = getKfcDir();
const coreDir = getCoreDir();
const extdirs = getExtensionDirs(true);

const extras = extdirs.map((fullpath) => {
  return {
    from: path.resolve(fullpath, 'dist'),
    to: 'app/kungfu-extensions',
  };
});

module.exports = {
  generateUpdatesFilesForAllChannels: true,
  appId: 'Kungfu.Origin.KungFu.Trader',
  electronVersion: kungfuCore.dependencies.electron,
  publish: [
    {
      provider: 'generic',
      url: 'https://www.kungfu-trader.com',
    },
  ],
  npmRebuild: false,
  files: [
    'dist/app/**/*',
    'dist/cli/**/*',
    '!**/@kungfu-trader/kfx-*/**/*',
    '!**/@kungfu-trader/kungfu-core/*',
    '!**/@kungfu-trader/kungfu-core/**/*',
    '**/@kungfu-trader/kungfu-core/lib/*',
    '**/@kungfu-trader/kungfu-core/*.json',
    '!**/@kungfu-trader/kungfu-app/*',
    '!**/@kungfu-trader/kungfu-app/**/*',
    '!**/@kungfu-trader/kungfu-cli/*',
    '!**/@kungfu-trader/kungfu-cli/**/*',
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
      to: 'app/dist/public/python',
      filter: ['*.whl'],
    },
    {
      from: appDir,
      to: 'app/dist',
      filter: ['public/config', 'public/key', 'public/logo', 'public/keywords'],
    },
    {
      from: appDir,
      to: 'app',
      filter: ['lib/*'],
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
