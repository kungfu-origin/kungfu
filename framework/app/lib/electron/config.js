const path = require('path');
const fse = require('fs-extra');
const os = require('os');
const kungfuCore = require('@kungfu-trader/kungfu-core/package.json');
const {
  getAppDir,
  getKfcDir,
  getCoreDir,
  getExtensionDirs,
  findPackageRoot,
} = require('@kungfu-trader/kungfu-js-api/toolkit/utils');

const isWindows = os.platform() === 'win32';
const appDir = getAppDir();
const vsDepsDir = path.join(appDir, 'public', 'vsDeps');
const kfcDir = getKfcDir();
const coreDir = getCoreDir();
const extensionDirs = getExtensionDirs(true);
const root = findPackageRoot();
console.log(`-- Package root ${root}`);
const logoDir = path.join(root, 'logo');
const logoPath = path.join(logoDir, 'logo-replace.png');
const dialogLogoPath = path.join(logoDir, 'dialog-logo-replace.png');
const icoLogoPath = path.join(logoDir, 'icon-replace.ico');
const icnsLogoPath = path.join(logoDir, 'icon-replace.icns');
const icoLogoPathResolved = fse.existsSync(icoLogoPath)
  ? icoLogoPath
  : `${appDir}/public/logo/icon.ico`;
const icnsLogoPathResolved = fse.existsSync(icnsLogoPath)
  ? icnsLogoPath
  : `${appDir}/public/logo/icon.icns`;
const languageDir = path.join(root, 'language');
const languageFile = path.join(languageDir, 'locale.json');
const languageCNMergeFile = path.join(languageDir, 'zh-CN-merge.json');
const languageENMergeFile = path.join(languageDir, 'en-US-merge.json');
const configDir = path.join(root, 'config');
const defaultInstrumentsJson = path.join(configDir, 'defaultInstruments.json');
const rootPackageJson = require(path.join(root, 'package.json'));
const appLibPackageJsonDir = path.join(appDir, 'lib', 'electron');
const appLibPackageMergeJson = require(path.join(
  appLibPackageJsonDir,
  'package.merge.json',
));
const appLibPackageJsonPath = path.join(appLibPackageJsonDir, 'package.json');

fse.writeJsonSync(appLibPackageJsonPath, {
  ...rootPackageJson,
  ...appLibPackageMergeJson,
});

const extensions = extensionDirs.map((fullpath) => {
  const extensionDir = path.resolve(fullpath, 'dist');
  console.log(
    `-- Found kungfu extension: [${fse.readdirSync(extensionDir).join(', ')}]`,
  );
  return {
    from: extensionDir,
    to: 'app/kungfu-extensions',
  };
});

if (fse.existsSync(languageFile)) {
  console.log(`-- Found language file ${languageFile}`);
}

if (fse.existsSync(languageCNMergeFile)) {
  console.log(`-- Found language cn merge file ${languageCNMergeFile}`);
}

if (fse.existsSync(languageENMergeFile)) {
  console.log(`-- Found language en merge file ${languageENMergeFile}`);
}

if (fse.existsSync(defaultInstrumentsJson)) {
  console.log(`-- Found defaultInstruments json ${defaultInstrumentsJson}`);
}

if (fse.existsSync(logoPath)) {
  console.log(`-- Found logo file ${logoPath}`);
}

if (fse.existsSync(dialogLogoPath)) {
  console.log(`-- Found dialog logo file ${dialogLogoPath}`);
}

if (fse.existsSync(icnsLogoPath)) {
  console.log(`-- Found icns logo file ${icnsLogoPath}`);
}

if (fse.existsSync(icoLogoPath)) {
  console.log(`-- Found ico logo file ${icoLogoPath}`);
}

module.exports = {
  generateUpdatesFilesForAllChannels: true,
  electronVersion:
    kungfuCore.devDependencies.electron || kungfuCore.dependencies.electron,
  publish: {
    provider: 'generic',
    url: 'www.kungfu-trader.com',
    channel: '${channel}-${os}',
  },
  npmRebuild: false,
  files: [
    'dist/app/**/*',
    'dist/cli/**/*',
    'dist/api/**/*',
    'dist/kfs/**/*',
    '!dist/kfs/native_modules/dist/**/*', //由于sdk依赖了app的electronBuilder方法, 所以会把electron打包进去, 需要过滤掉
    '!**/@kungfu-trader/kfx-*/**/*',
    '!**/@kungfu-trader/kungfu-sdk/**/*',
    '**/@kungfu-trader/kungfu-sdk/templates/**/*',
    '!**/@kungfu-trader/kungfu-core/*',
    '!**/@kungfu-trader/kungfu-core/**/*',
    '**/@kungfu-trader/kungfu-core/lib/*',
    '**/@kungfu-trader/kungfu-core/*.json',
    '!**/@kungfu-trader/kungfu-app/*',
    '!**/@kungfu-trader/kungfu-app/**/*',
    '**/@kungfu-trader/kungfu-app/lib/*',
    '**/@kungfu-trader/kungfu-app/package.json',
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
    ...(isWindows // 只在 windows 下拷贝 vs 依赖到 kfc
      ? [
          {
            from: vsDepsDir,
            to: 'kfc',
          },
        ]
      : []),
    {
      from: `${coreDir}/build/python/dist`,
      to: 'app/dist/public/python',
      filter: ['*.whl'],
    },
    {
      from: appDir,
      to: 'app/dist',
      filter: [
        'public/config',
        'public/key',
        'public/logo',
        'public/keywords',
        'public/music',
        'public/language',
        ...(fse.existsSync(defaultInstrumentsJson)
          ? ['!public/config/defaultInstruments.json']
          : []),
      ],
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
    ...(fse.existsSync(languageDir)
      ? [
          {
            from: languageDir,
            to: 'app/dist/public/language',
            filter: ['locale.json', 'zh-CN-merge.json', 'en-US-merge.json'],
          },
        ]
      : []),
    ...(fse.existsSync(defaultInstrumentsJson)
      ? [
          {
            from: defaultInstrumentsJson,
            to: 'app/dist/public/config/defaultInstruments.json',
          },
        ]
      : []),
    ...(fse.existsSync(logoPath)
      ? [
          {
            from: logoDir,
            to: 'app/dist/public/logo',
            filter: ['*.png', '*.ico', '*.icns'],
          },
        ]
      : []),
    ...extensions,
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
    icon: icnsLogoPathResolved,
    type: 'distribution',
    target: ['dmg', 'zip'],
  },
  win: {
    icon: icoLogoPathResolved,
    target: [
      {
        target: 'nsis',
        arch: ['x64'],
      },
      {
        target: 'zip',
        arch: ['x64'],
      },
    ],
  },
  linux: {
    icon: icnsLogoPathResolved,
    target: ['rpm', 'appimage', 'zip'],
    executableName: 'Kungfu.app',
  },
  nsis: {
    oneClick: false,
    allowElevation: true,
    allowToChangeInstallationDirectory: true,
    installerIcon: icoLogoPathResolved,
    uninstallerIcon: icoLogoPathResolved,
    installerHeaderIcon: icoLogoPathResolved,
    createDesktopShortcut: true,
    createStartMenuShortcut: true,
  },

  afterAllArtifactBuild: (result) => {
    if (process.env.CI && process.platform === 'win32') {
      fse.removeSync(path.join(result.outDir, 'win-unpacked'));
    }
    return result.artifactPaths;
  },
};
