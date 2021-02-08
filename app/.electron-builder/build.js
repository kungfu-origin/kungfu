const builder = require('electron-builder');
const fse = require('fs-extra');
const path = require('path');
const minimist = require('minimist');
const moment = require('moment')

const baseConfig = fse.readJSONSync(path.join(__dirname, 'base.json'))


const argv = minimist(process.argv, {
    boolean: [ 'cli', 'pro' ],
    string: 'target'
})
const { cli, pro, target } = argv

console.log('argv', argv)

if (pro) {
    baseConfig.extraResources.push({
        "from": path.join(__dirname, '..', '..', 'kfext_task_timerTrader', 'lib'),
        "to": "kungfu-extensions/timer-trader"
    })
}

if (target) {
    const targetIndex = findConfigItemIndex(baseConfig.extraResources, [{
        key: 'from',
        value: '../core/build/kfc',
    },{
        key: 'to',
        value: 'kfc'
    }])
    
    if (targetIndex >= 0) {
        baseConfig.extraResources[targetIndex] = {
            "from": "../core/build/kfc",
            "to": "kfc",
            "filter": [
                "!**/btdata",
                "!**/kungfu_extensions/tora",
                "!**/kungfu_extensions/shengli",
                "!**/kungfu_extensions/xtp"
            ]
        }
    }
}

switch (target) {
    case 'tora':
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_tora', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/tora"
        })
        break
    case 'shengli':
        baseConfig.extraResources.push({
            "from": path.join(__dirname, '..', '..', 'kfext_shengli', 'build', 'dist'),
            "to": "kfc/kungfu_extensions/shengli"
        })
        break
}

baseConfig.artifactName = resolveArtifactName(cli, pro, target)
builder.build({
    config: baseConfig
})


//conditions: { key: '', value: '' }
function findConfigItemIndex (configList, conditions) {
    for(let i = 0; len = configList.length; i++) {
        const item = configList[i];
        const matchList = conditions.filter(c => {
            if (item[c.key] === c.value) return true;
        })

        if (matchList.length === conditions.length) {
            return i 
        }
    }
}


function resolveArtifactName (cli, pro, target) {
    const buildTime = moment().format('YYYYMMDDHHMMSS')
    const appType = cli ? 'cli' : 'app';
    const targetName = target || '';
    const appTypeResolvde = pro ? `${appType}-pro` : appType

    const specialName = [ appTypeResolvde, targetName, buildTime ]
        .filter(n => !!n)
        .join('-')

    return '${productName}-${buildVersion}-${os}-${arch}-${channel}' + `-${specialName}` + '.${ext}'
}