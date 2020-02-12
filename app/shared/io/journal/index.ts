import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/busiUtils'; 

const kungfu = require('kungfu-core').kungfu;

export const locator = kungfu.locator(KF_HOME);
export const watcher = kungfu.watcher(locator);
// export const ledger = watcher.ledger;

// watcher.setup();
// setTimerPromiseTask(() => {
//     if (watcher.isLive()) {
//         watcher.step();
//         console.log('ledger state: ', watcher.ledger);
//         console.log('ledger config: ', watcher.ledger.Config);
//     } else {
//         process.exit();
//     }
// }, 1000);