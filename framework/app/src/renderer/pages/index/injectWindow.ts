import workers from '@kungfu-trader/kungfu-app/src/renderer/assets/workers';
import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { kf } from '@kungfu-trader/kungfu-js-api/kungfu';

window.workers = workers;
window.watcher = watcher;
window.kungfu = kf;
