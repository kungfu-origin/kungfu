
import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/BusiUtils';

process.env.KF_LOG_LEVEL = 'trace';

export const kungfu = require('kungfu-core').kungfu;
export const watcher = kungfu.watcher(KF_HOME, `watcher_${process.env.APP_TYPE}`)

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
                callback(Object.freeze(watcher.state));
            }

            resolve();
        })
    }, interval);
}





