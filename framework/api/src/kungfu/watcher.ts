import { kf } from './index';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
  booleanProcessEnv,
  getProcessIdByKfLocation,
  kfLogger,
  setTimerPromiseTask,
  // statTime,
  // statTimeEnd,
} from '../utils/busiUtils';
import {
  BrokerStateStatusEnum,
  BrokerStateStatusTypes,
} from '../typings/enums';

export const watcher = ((): KungfuApi.Watcher | null => {
  kfLogger.info(
    'Init Watcher',
    'APP_TYPE',
    process.env.APP_TYPE,
    'RENDERER_TYPE',
    process.env.RENDERER_TYPE,
  );

  if (process.env.APP_TYPE !== 'renderer') {
    return null;
  }

  if (process.env.RENDERER_TYPE !== 'app') {
    return null;
  }

  const id = [process.env.APP_TYPE || '', process.env.RENDERER_TYPE || ''].join(
    '',
  );
  const bypassRestore = booleanProcessEnv(process.env.RELOAD_AFTER_CRASHED);

  return kf.watcher(
    KF_RUNTIME_DIR,
    kf.formatStringToHashHex(id),
    false,
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

export const startUpdateKungfuWatcherQuotes = (interval = 2000) => {
  return;
  if (watcher === null) return;

  return setTimerPromiseTask(() => {
    return new Promise((resolve) => {
      if (watcher.isLive()) {
        if (process.env.APP_TYPE == 'renderer') {
          window.requestIdleCallback(
            () => {
              // statTime('update Quote');
              watcher.updateQuote();
              // statTimeEnd('update Quote');
              resolve(true);
            },
            { timeout: 5000 },
          );
        } else {
          watcher.updateQuote();
          resolve(true);
        }
      } else {
        resolve(false);
      }
    });
  }, interval);
};

export const dealAppStates = (
  appStates: Record<string, BrokerStateStatusEnum>,
): Record<string, BrokerStateStatusTypes> => {
  if (!watcher) {
    return {} as Record<string, BrokerStateStatusTypes>;
  }

  return Object.keys(appStates || {}).reduce((appStatesResolved, key) => {
    const processId = watcher.getProcessId(key);
         console.log(processId);
    const appStateValue = appStates[key] as BrokerStateStatusEnum;
         console.log(appStateValue);
    appStatesResolved[processId] = BrokerStateStatusEnum[
      appStateValue
    ] as BrokerStateStatusTypes;
         console.log(appStatesResolved[processId]);
    return appStatesResolved;
  }, {} as Record<string, BrokerStateStatusTypes>);
};

export const dealAssetsByHolderUID = (
  assets: KungfuApi.DataTable<KungfuApi.Asset>,
): Record<string, KungfuApi.Asset> => {
  if (!watcher) {
    return {} as Record<string, KungfuApi.Asset>;
  }

  return Object.values(assets).reduce((assetsResolved, asset) => {
         console.log(asset);
    const { holder_uid } = asset;
         console.log("dealAssetsByHolderUID 444");
    const processId = watcher.getProcessId(holder_uid);
         console.log("dealAssetsByHolderUID 666");
         console.log(processId);
    assetsResolved[processId] = asset;
         console.log("dealAssetsByHolderUID 777");
    return assetsResolved;
  }, {} as Record<string, KungfuApi.Asset>);
};
