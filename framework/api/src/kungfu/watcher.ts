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

export const watcher = ((): KungfuApi.Watcher | null => {
  kfLogger.info(
    'Init Watcher',
    'APP_TYPE',
    process.env.APP_TYPE || 'undefined',
    'RENDERER_ID',
    process.env.RENDERER_ID || 'undefined',
    'DAEMON_ID',
    process.env.DAEMON_ID || 'undefined',
  );

  console.log(
    'Init Watcher',
    'APP_TYPE',
    process.env.APP_TYPE || 'undefined',
    'RENDERER_ID',
    process.env.RENDERER_ID || 'undefined',
    'DAEMON_ID',
    process.env.DAEMON_ID || 'undefined',
  );

  if (process.env.APP_TYPE !== 'renderer') {
    if (process.env.APP_TYPE !== 'cli') {
      if (process.env.APP_TYPE !== 'daemon') {
        return null;
      }
    }
  }

  if (process.env.APP_TYPE === 'renderer') {
    if (process.env.RENDERER_ID !== 'app') {
      return null;
    }
  }

  if (process.env.APP_TYPE === 'cli') {
    if (process.env.RENDERER_ID !== 'dzxy') {
      return null;
    }
  }

  const id = [
    process.env.APP_TYPE || '',
    process.env.DAEMON_ID || process.env.RENDERER_ID || '',
  ].join('');

  const bypassRestore = booleanProcessEnv(
    process.env.RELOAD_AFTER_CRASHED || process.env.BY_PASS_RESTORE || '',
  );
  const globalSetting = getKfGlobalSettingsValue();
  const bypassQuote = globalSetting?.performance?.bypassQuote;

  return kf.watcher(
    KF_RUNTIME_DIR,
    kf.formatStringToHashHex(id),
    bypassQuote,
    bypassRestore,
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
