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
    'RENDERER_TYPE',
    process.env.RENDERER_TYPE || 'undefined',
    'DAEMON_TYPE',
    process.env.DAEMON_TYPE || 'undefined',
  );

  if (process.env.APP_TYPE !== 'renderer') {
    if (process.env.APP_TYPE !== 'cli') {
      if (process.env.APP_TYPE !== 'daemon') {
        return null;
      }
    }
  }

  if (process.env.APP_TYPE === 'renderer') {
    if (process.env.RENDERER_TYPE !== 'app') {
      return null;
    }
  }

  if (process.env.APP_TYPE === 'cli') {
    if (process.env.RENDERER_TYPE !== 'dzxy') {
      if (process.env.RENDERER_TYPE !== 'export') {
        return null;
      }
    }
  }

  const id = [
    process.env.APP_TYPE || '',
    process.env.RENDERER_TYPE || process.env.DAEMON_TYPE || '',
  ].join('');

  const bypassRestore =
    process.env.APP_TYPE === 'daemon'
      ? false
      : booleanProcessEnv(process.env.RELOAD_AFTER_CRASHED || '');
  const globalSetting = getKfGlobalSettingsValue();
  const bypassQuote = globalSetting?.performance?.bypassQuote;

  return kf.watcher(
    KF_RUNTIME_DIR,
    kf.formatStringToHashHex(id),
    bypassQuote,
    bypassRestore,
  );
})();

export const startStep = () => {
  if (watcher === null) return;
  watcher.createTask();
};

export const startGetKungfuWatcherStep = (
  interval = 1000,
  callback: (watcher: KungfuApi.Watcher) => void,
) => {
  if (watcher === null) return;

  return setTimerPromiseTask(() => {
    return new Promise((resolve) => {
      if (watcher.isLive()) {
        callback(watcher);
      }
      resolve(true);
    });
  }, interval);
};
