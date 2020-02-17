import { setTimerPromiseTask } from '__gUtils/busiUtils'; 
import { watcher } from '__gUtils/kungfuUtils';


watcher.setup();
setTimerPromiseTask(() => {
    return new Promise((resolve) => {
        console.log('test watcher');
        if (watcher.isLive()) {
            console.log('watcher is live')
            watcher.step();
            console.log('ledger state: ', watcher.ledger);
        }
        resolve()
    })
}, 5000);