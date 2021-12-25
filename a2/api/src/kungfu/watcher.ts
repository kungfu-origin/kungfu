import { kf } from './index';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
    getProcessIdByKfLocation,
    kfLogger,
    setTimerPromiseTask,
} from '../utils/busiUtils';
import { BrokerStateStatusEnum, BrokerStateStatusTypes } from '../typings';

export const watcher = ((): Watcher | null => {
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

    const id = [
        process.env.APP_TYPE || '',
        process.env.RENDERER_TYPE || '',
    ].join('');
    const bypassRestore = process.env.RELOAD_AFTER_CRASHED ? true : false;

    return kf.watcher(
        KF_RUNTIME_DIR,
        kf.formatStringToHashHex(id),
        false,
        bypassRestore,
    );
})();

export const startGetKungfuWatcherStep = (
    interval = 1000,
    callback: (watcher: Watcher) => void,
) => {
    if (watcher === null) return;

    return setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (
                !watcher.isLive() &&
                !watcher.isStarted() &&
                watcher.isUsable()
            ) {
                watcher.setup();
                callback(watcher);
            }

            if (watcher.isLive()) {
                if (process.env.APP_TYPE == 'renderer') {
                    window.requestIdleCallback(
                        () => {
                            // statTime('step');
                            watcher.step();
                            callback(watcher);
                            // statTimeEnd('step');
                            resolve(true);
                        },
                        { timeout: 5000 },
                    );
                } else {
                    watcher.step();
                    callback(watcher);
                    resolve(true);
                }
            }

            resolve(true);
        });
    }, interval);
};

export const startUpdateKungfuWatcherQuotes = (interval = 2000) => {
    if (watcher === null) return;

    return setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (
                !watcher.isLive() ||
                !watcher.isStarted() ||
                !watcher.isUsable()
            ) {
                resolve(false);
                return;
            }

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
            }
        });
    }, interval);
};

export const dealAppStates = (
    appStates: Record<string, BrokerStateStatusEnum>,
): Record<string, BrokerStateStatusTypes> => {
    const appStatesResolved: Record<string, BrokerStateStatusTypes> = {};

    if (!watcher) {
        return appStatesResolved;
    }

    Object.keys(appStates).forEach((key: string) => {
        const kfLocation = watcher.getLocation(key);
        const processId = getProcessIdByKfLocation(kfLocation);
        const appStateValue = appStates[key] as BrokerStateStatusEnum;
        appStatesResolved[processId] = BrokerStateStatusEnum[
            appStateValue
        ] as BrokerStateStatusTypes;
    });

    return appStatesResolved;
};

export const dealAssetsByHolderUID = (
    assets: DataTable<Asset>,
): Record<string, Asset> => {
    const assetsResolved: Record<string, Asset> = {};

    if (!watcher) {
        return assetsResolved;
    }

    Object.values(assets).forEach((asset: Asset) => {
        const { holder_uid } = asset;
        const kfLocation = watcher.getLocation(holder_uid);
        const processId = getProcessIdByKfLocation(kfLocation);
        assetsResolved[processId] = asset;
    });

    return assetsResolved;
};
