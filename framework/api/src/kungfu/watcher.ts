import { kf } from './index';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import { getKfGlobalSettingsValue } from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  booleanProcessEnv,
  kfLogger,
  setTimerPromiseTask,
  // statTime,
  // statTimeEnd,
} from '../utils/busiUtils';

export const getWatcherId = () => {
  const watcherId = [
    process.env.APP_TYPE,
    process.env.UI_EXT_TYPE,
    (process.env.APP_ID || '').length > 16
      ? kf.formatStringToHashHex(process.env.APP_ID || '')
      : process.env.APP_ID,
  ]
    .filter((str) => !!str)
    .join('-');
  kfLogger.info(`WatcherId ${watcherId}`);
  return watcherId;
};

export const watcher = ((): KungfuApi.Watcher | null => {
  if (process.env.APP_TYPE !== 'renderer') {
    if (process.env.APP_TYPE !== 'daemon') {
      if (process.env.APP_TYPE !== 'cli') {
        return null;
      }
    }
  }

  if (process.env.APP_TYPE === 'renderer') {
    if (process.env.APP_ID !== 'app') {
      return null;
    }
  }

  kfLogger.info(
    'Init Watcher',
    'APP_TYPE',
    process.env.APP_TYPE || 'undefined',
    'UI_EXT_TYPE',
    process.env.UI_EXT_TYPE || 'undefined',
    'APP_ID',
    process.env.APP_ID || 'undefined',
  );

  const bypassRestore =
    booleanProcessEnv(process.env.RELOAD_AFTER_CRASHED) ||
    process.env.BY_PASS_RESTORE;
  const globalSetting = getKfGlobalSettingsValue();
  const bypassAccounting =
    process.env.BY_PASS_ACCOUNTING ??
    globalSetting?.performance?.bypassAccounting ??
    false;
  const bypassTradingData =
    process.env.BY_PASS_TRADINGDATA ??
    globalSetting?.performance?.bypassTradingData ??
    false;
  const refreshTradingDataBeforeSync =
    process.env.REFRESH_LEDGER_BEFORE_SYNC ?? false;

  const millisecondsSleepAfterStep =
    process.env.MILLISECONDS_SLEEP_AFTER_STEP ?? 100;

  kfLogger.info('bypassRestore', bypassRestore);
  kfLogger.info('bypassAccounting', bypassAccounting);
  kfLogger.info('bypassTradingData', bypassTradingData);
  kfLogger.info('refreshTradingDataBeforeSync', refreshTradingDataBeforeSync);
  kfLogger.info('millisecondsSleepAfterStep', millisecondsSleepAfterStep);

  return kf.watcher(
    KF_RUNTIME_DIR,
    getWatcherId(),
    !!bypassRestore,
    !!bypassAccounting,
    !!bypassTradingData,
    !!refreshTradingDataBeforeSync,
    +millisecondsSleepAfterStep,
  );
})();

export const startWatcher = () => {
  if (watcher === null) return;
  watcher.start();
};

export const startWatcherSyncTask = (
  interval = 1000,
  callback?: (watcher: KungfuApi.Watcher) => void,
) => {
  if (watcher === null) return;
  return setTimerPromiseTask(async () => {
    if (watcher.isLive() && watcher.isStarted()) {
      watcher.sync();
      callback && (await callback(watcher));
    }
    return true;
  }, interval);
};
