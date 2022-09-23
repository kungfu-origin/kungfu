import { kf } from '@kungfu-trader/kungfu-js-api/kungfu';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-js-api//config/pathConfig';
import { getKfGlobalSettingsValue } from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import {
  booleanProcessEnv,
  setTimerPromiseTask,
} from '@kungfu-trader/kungfu-js-api//utils/busiUtils';

const IS_CLI_DEV =
  process.env.APP_TYPE !== 'cli' || process.env.NODE_ENV === 'development';

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
  IS_CLI_DEV && console.log(`WatcherId ${watcherId}`);
  return watcherId;
};

export const getWatcher = (): KungfuApi.Watcher | null => {
  if (process.env.APP_TYPE !== 'cli') {
    return null;
  }

  //for cli show
  IS_CLI_DEV &&
    console.log(
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
    globalSetting?.performance?.bypassAccounting;
  const bypassTradingData =
    process.env.BY_PASS_TRADINGDATA ??
    globalSetting?.performance?.bypassTradingData;

  if (IS_CLI_DEV) {
    console.log('bypassRestore', bypassRestore);
    console.log('bypassAccounting', bypassAccounting);
    console.log('bypassTradingData', bypassTradingData);
  }

  return kf.watcher(
    KF_RUNTIME_DIR,
    getWatcherId(),
    !!bypassRestore,
    !!bypassAccounting,
    !!bypassTradingData,
  );
};

export const startWatcher = (watcher: KungfuApi.Watcher) => {
  if (watcher === null) return;
  watcher.start();
};

export const startWatcherSyncTask = (
  watcher: KungfuApi.Watcher,
  interval = 1000,
  callback?: (watcher: KungfuApi.Watcher) => void,
) => {
  if (watcher === null) return;
  return setTimerPromiseTask(() => {
    return new Promise((resolve) => {
      if (watcher.isLive() && watcher.isStarted()) {
        watcher.sync();
        callback && callback(watcher);
      }
      resolve(true);
    });
  }, interval);
};

export const triggerStartStep = (
  watcher: KungfuApi.Watcher | null,
  stepInterval = 2000,
) => {
  if (watcher) {
    startWatcher(watcher);
    startWatcherSyncTask(watcher, stepInterval, (_) => {
      console.log('triggerStartStep');
    });
  }
};
