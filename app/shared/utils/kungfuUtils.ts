
import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask, encodeKeyToKungfuKey } from '__gUtils/BusiUtils';

process.env.KF_LOG_LEVEL = 'trace';


console.log(11111)

export const kungfu = require('kungfu-core').kungfu;
console.log(2222)

export const watcher = kungfu.watcher(KF_HOME, `watcher_${process.env.APP_TYPE}`)
console.log(33333)
export const startGetKungfuState = (callback: Function, interval = 1000) => {
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (watcher.isUsable() && !watcher.isLive() && !watcher.isStarted()) {
                watcher.setup();
            }
            if (watcher.isLive()) {
                watcher.step();
            }
            if (watcher.isLive() && watcher.isStarted()) {
                callback(Object.freeze(watcher.ledger));
            }

            resolve();
        })
    }, interval);
}

console.log(4444)

const kungfuConfigStore = kungfu.ConfigStore(KF_HOME);
console.log(5555)

export const getAllKfConfig = () => {
    return kungfuConfigStore.getAllConfig();
}

export const setKfConfig = (key: string, type: string, config: string) => {
    const kungfuKey = encodeKeyToKungfuKey(key, type);
    return kungfuConfigStore.setConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3], config)
}

export const getKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKeyToKungfuKey(key, type);
    return kungfuConfigStore.getConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3])    
}

export const removeKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKeyToKungfuKey(key, type);
    return kungfuConfigStore.removeConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3])
}

