
import { KF_HOME } from '__gConfig/pathConfig';

process.env.KF_LOG_LEVEL = 'trace'
console.log(process.env.KF_LOG_LEVEL)

const kungfu = require('kungfu-core').kungfu;
const locator = kungfu.locator(KF_HOME);

export const watcher = kungfu.watcher(locator, `watcher_${process.env.APP_TYPE}`)

declare global {
    interface Window { 
        __watcher: any;
        __locator: any;
    }
}

export {}

if (process.env.APP_TYPE === 'renderer') {
    window.__watcher = watcher;
    window.__locator = locator;
} else {
    global.__watcher = watcher;
    global.__locator = locator;
}