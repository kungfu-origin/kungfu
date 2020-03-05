
import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask, encodeKeyToKungfuLocation } from '__gUtils/BusiUtils';

process.env.KF_LOG_LEVEL = 'trace';

export const kungfu = require('kungfu-core').kungfu;
export const watcher = kungfu.watcher(KF_HOME, `watcher_${process.env.APP_TYPE}`)
export const startGetKungfuState = (callback: Function, interval = 1000) => {
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }
            
            if (watcher.isLive()) {
                watcher.step();
            }
            
            callback(Object.freeze(watcher.ledger));
            resolve();
        })
    }, interval);
}

const kungfuConfigStore = kungfu.ConfigStore(KF_HOME);

export const getAllKfConfig = () => {
    return kungfuConfigStore.getAllConfig();
}

export const setKfConfig = (key: string, type: string, config: string) => {
    const kungfuKey = encodeKeyToKungfuLocation(key, type);
    return kungfuConfigStore.setConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3], config)
}

export const getKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKeyToKungfuLocation(key, type);
    return kungfuConfigStore.getConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3])    
}

export const removeKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKeyToKungfuLocation(key, type);
    return kungfuConfigStore.removeConfig(kungfuKey[0], kungfuKey[1], kungfuKey[2], kungfuKey[3])
}

