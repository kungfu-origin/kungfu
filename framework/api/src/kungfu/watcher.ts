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
  return `${process.env.APP_TYPE || ''}-${
    process.env.UI_EXT_TYPE || ''
  }-${kf.formatStringToHashHex(process.env.APP_ID || '')}`;
};

export const watcher = ((): KungfuApi.Watcher | null => {
  kfLogger.info(
    'Init Watcher',
    'APP_TYPE',
    process.env.APP_TYPE || 'undefined',
    'UI_EXT_TYPE',
    process.env.UI_EXT_TYPE || 'undefined',
    'APP_ID',
    process.env.APP_ID || 'undefined',
  );

  if (process.env.APP_TYPE !== 'renderer') {
    if (process.env.APP_TYPE !== 'daemon') {
      return null;
    }
  }

  if (process.env.APP_TYPE === 'renderer') {
    if (process.env.APP_ID !== 'app') {
      return null;
    }
  }

  const bypassRestore = booleanProcessEnv(
    process.env.RELOAD_AFTER_CRASHED || process.env.BY_PASS_RESTORE || '',
  );

  const globalSetting = getKfGlobalSettingsValue();
  const bypassAccounting = globalSetting?.performance?.bypassAccounting;
  const bypassTradingData = globalSetting?.performance?.bypassTradingData;
  console.log('bypassRestore', bypassRestore);
  console.log('bypassAccounting', bypassAccounting);
  console.log('bypassTradingData', bypassTradingData);

  return kf.watcher(
    KF_RUNTIME_DIR,
    getWatcherId(),
    !!bypassRestore,
    !!bypassAccounting,
    !!bypassTradingData,
  );
})();

export const startWatcher = () => {
  if (watcher === null) return;
  watcher.start();
};

export const startWatcherSyncTask = (
  interval = 1000,
  callback: (watcher: KungfuApi.Watcher) => void,
) => {
  if (watcher === null) return;
  return setTimerPromiseTask(() => {
    return new Promise((resolve) => {
      if (watcher.isLive()) {
        watcher.sync();
        callback(watcher);
      }
      resolve(true);
    });
  }, interval);
};
