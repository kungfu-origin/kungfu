const builder = require('electron-builder');
const fse = require('fs-extra');
const path = require('path');
const minimist = require('minimist');
const moment = require('moment');
const semver = require('semver');

const appConfig = fse.readJSONSync(path.join(path.dirname(__dirname), 'package.json'))
const baseConfig = fse.readJSONSync(path.join(__dirname, 'base.json'))

const argv = minimist(process.argv, {
    boolean: ['pro'],
    string: 'target'
})
const {pro, target} = argv

console.log('argv', argv)

if (pro) {
    baseConfig.extraResources.push({
        "from": path.join(__dirname, '..', '..', 'kfext_task_timerTrader', 'lib'),
        "to": "kungfu-extensions/timer-trader"
    })

    baseConfig.extraResources.push({
        "from": path.join(__dirname, '..', '..', 'kfext_task_backwards', 'lib'),
        "to": "kungfu-extensions/backwards"
    })
}

if (target) {
    const targetIndex = findConfigItemIndex(baseConfig.extraResources, [{
        key: 'from',
        value: '../core/dist/kfc',
    }, {
        key: 'to',
        value: 'kfc'
    }])

    if (targetIndex >= 0) {
        baseConfig.extraResources[targetIndex] = {
            "from": "../core/dist/kfc",
            "to": "kfc",
            "filter": [
                "!**/btdata",
                target.includes('xtp') ? "" : "!**/kungfu_extensions/xtp",
                hasFuture(target) ? "!**/kungfu_extensions/ctp" : "",
            ].filter(key => !!key)
        }
    }

    if (target.includes('tora')) {
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_tora', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/tora"
        })
    }

    if (target.includes('zhaos')) {
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_zhaos', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/zhaos",
        })
    }

    if (target.includes('shengli')) {
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_shengli', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/shengli"
        })
    }

    if (target.includes('rongh')) {
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_rongh', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/rongh"
        })
    }
}

const version = semver.parse(appConfig.version);

baseConfig.directories.output = path.join("build", "stage", "kungfu-app", `v${version.major}`, `v${version}`);
baseConfig.artifactName = resolveArtifactName(pro, target)
builder.build({
    config: baseConfig
})


function hasFuture(target) {
    if (target.includes('zhaos')) return true;
    if (target.includes('rongh')) return true;
    return false
}

//conditions: { key: '', value: '' }
function findConfigItemIndex(configList, conditions) {
    for (let i = 0; len = configList.length; i++) {
        const item = configList[i];
        const matchList = conditions.filter(c => {
            if (item[c.key] === c.value) return true;
        })

        if (matchList.length === conditions.length) {
            return i
        }
    }
}


function resolveArtifactName(pro, target) {
    const buildTime = moment().format('MMDDHHmm')
    const appType = 'app';
    const targetName = target || '';
    const appTypeResolved = pro ? `${appType}-pro` : appType

    const specialName = [appTypeResolved, targetName, buildTime]
        .filter(n => !!n)
        .join('-')

    return '${productName}-${buildVersion}-${os}-${arch}-${channel}' + `-${specialName}` + '.${ext}'
}