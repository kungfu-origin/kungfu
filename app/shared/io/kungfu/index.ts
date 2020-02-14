import { setTimerPromiseTask } from '__gUtils/busiUtils'; 
import { watcher } from '__gUtils/dbUtils';

console.log(watcher, '-===')

// renderWatcher.setup();
// setTimerPromiseTask(() => {
//     return new Promise((resolve) => {
//         if (renderWatcher.isLive()) {
//             renderWatcher.step();
//             // console.log('ledger state: ', renderWatcher.ledger);
//         }
//         resolve()
//     })
// }, 1000);