import workers from '@kungfu-trader/kungfu-app/src/renderer/assets/workers';
import {
  getWatcherId,
  watcher,
} from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import {
  basketInstrumentStore,
  basketStore,
  commissionStore,
  configStore,
  kf,
  riskSettingStore,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-js-api/config/pathConfig';

window.workers = workers;
window.watcher = watcher;
window.kungfu = kf;
window.basketStore = basketStore;
window.basketInstrumentStore = basketInstrumentStore;
window.configStore = configStore;
window.riskSettingStore = riskSettingStore;
window.commissionStore = commissionStore;

window.testCase = {};
window.testCase.crashTheWatcher = () => {
  const id = getWatcherId();
  return kf.watcher(KF_RUNTIME_DIR, kf.formatStringToHashHex(id), false, false);
};
