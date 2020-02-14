import { setTimerPromiseTask } from '__gUtils/busiUtils'; 
import { watcher } from '__gUtils/dbUtils';


watcher.setup();
setTimerPromiseTask(() => {
    return new Promise((resolve) => {
        if (watcher.isLive()) {
            watcher.step();
            console.log('ledger state: ', watcher.ledger);
        }
        resolve()
    })
}, 5000);