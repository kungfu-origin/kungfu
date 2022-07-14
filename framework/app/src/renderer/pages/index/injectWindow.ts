import workers from '@kungfu-trader/kungfu-app/src/renderer/assets/workers';
import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { kf } from '@kungfu-trader/kungfu-js-api/kungfu';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import pm2 from '@kungfu-trader/kungfu-js-api/utils/pm2Custom';

window.workers = workers;
window.watcher = watcher;
window.kungfu = kf;
window.pm2 = pm2;

window.testCase = {};
window.testCase.crashTheWatcher = () => {
  const id = [
    process.env.APP_TYPE || '',
    process.env.DAEMON_ID || process.env.RENDERER_ID || '',
  ].join('');

  return kf.watcher(KF_RUNTIME_DIR, kf.formatStringToHashHex(id), false, false);
};
