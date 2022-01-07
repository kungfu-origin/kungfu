const path = require('path');
const fse = require('fs-extra');
const semver = require('semver');
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

console.log(appDir, coreDir, kfcDir);

module.exports = {
    productName: 'Kungfu',
    artifactName:
        '${productName}-${buildVersion}-${os}-${arch}-${channel}.${ext}',
    generateUpdatesFilesForAllChannels: true,
    appId: 'Kungfu.Origin.KungFu.Trader',
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
        {
            from: appDir,
            to: 'dist/app',
            filter: ['public'],
        },
        '!**/@kungfu-trader/kungfu-core/*',
        '!**/@kungfu-trader/kungfu-core/**/*',
        '**/@kungfu-trader/kungfu-core/lib/*',
        '**/@kungfu-trader/kungfu-core/*.json',
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
            from: `${appDir}/public`,
            to: 'kungfu-resources',
            filter: ['config/*', 'key/*'],
        },
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
        icon: `${appDir}/public/logo/icn.icns`,
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
