const builder = require('electron-builder');
const path = require('path');
const minimist = require('minimist');
const moment = require('moment');
const baseConfig = require('./config');

function findConfigItemIndex(configList, conditions) {
    for (let i = 0; (len = configList.length); i++) {
        const item = configList[i];
        const matchList = conditions.filter((c) => {
            if (item[c.key] === c.value) return true;
        });

        if (matchList.length === conditions.length) {
            return i;
        }
    }
}

function resolveArtifactName(cli, pro, target) {
    const buildTime = moment().format('MMDDHHmm');
    const appType = cli ? 'cli' : 'app';
    const targetName = target || '';
    const appTypeResolvde = pro ? `${appType}-pro` : appType;

    const specialName = [appTypeResolvde, targetName, buildTime]
        .filter((n) => !!n)
        .join('-');

    return (
        '${productName}-${buildVersion}-${os}-${arch}-${channel}' +
        `-${specialName}` +
        '.${ext}'
    );
}

const run = (distDir) => {
    const pkgDir = path.dirname(distDir);
    process.chdir(pkgDir);

    const argv = minimist(process.argv, {
        boolean: ['pro'],
        string: 'target',
    });
    const { pro, target } = argv;

    console.log('argv', argv);

    if (target) {
        const targetIndex = findConfigItemIndex(baseConfig.extraResources, [
            {
                key: 'from',
                value: '../core/dist/kfc',
            },
            {
                key: 'to',
                value: 'kfc',
            },
        ]);

        if (targetIndex >= 0) {
            baseConfig.extraResources[targetIndex] = {
                from: '../core/dist/kfc',
                to: 'kfc',
                filter: ['!**/btdata'].filter((key) => !!key),
            };
        }
    }

    baseConfig.artifactName = resolveArtifactName(pro, target);

    return builder.build({
        config: baseConfig,
    });
};

module.exports = run;

if (require.main === module) {
    run(require('@kungfu-trader/kungfu-app').defaultDistDir).catch(
        console.error,
    );
}
