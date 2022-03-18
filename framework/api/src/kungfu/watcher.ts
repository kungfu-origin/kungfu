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
  );

  if (process.env.APP_TYPE !== 'renderer') {
    if (process.env.APP_TYPE !== 'cli') {
      return null;
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

  const id = [process.env.APP_TYPE || '', process.env.RENDERER_TYPE || ''].join(
    '',
  );

  const bypassRestore = booleanProcessEnv(
    process.env.RELOAD_AFTER_CRASHED || '',
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

export const startStep = (
) => {
  watcher.createTask();
};

export const startGetKungfuWatcherStep = (
  interval = 1000,
  callback: (watcher: KungfuApi.Watcher) => void,
) => {
  // return;
  if (watcher === null) return;

  return setTimerPromiseTask(() => {
    return new Promise((resolve) => {
      // if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
      //   watcher.setup();
      //   callback(watcher);
      // }

      // if (watcher.isLive()) {
      //   if (process.env.APP_TYPE == 'renderer') {
      //     window.requestIdleCallback(
      //       () => {
      //         // statTime('step');
      //         // watcher.step();
      //         callback(watcher);
      //         // statTimeEnd('step');
      //         resolve(true);
      //       },
      //       { timeout: 5000 },
      //     );
      //   } else {
      //     // watcher.step();
      //     callback(watcher);
      //     resolve(true);
      //   }
      // }
      console.log(watcher.isLive());
        // watcher.lock();
      if (watcher.isLive()) {
        console.log(watcher.appStates);
        console.log(watcher.ledger);
        console.log(watcher);
        callback(watcher);
        console.log("222222222");
      }
        // watcher.unlock();

      resolve(true);
    });
  }, interval);
};
