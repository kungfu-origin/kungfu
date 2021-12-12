import path from 'path';
import { getKfAllConfig, removeKfConfig } from '../kungfu/store';
import {
    KfConfig,
    KfLocation,
    KfModeTypes,
    KfCategoryEnum,
    KfCategoryTypes,
    KfModeEnum,
    KfConfigOrigin,
} from '../typings';
import { KF_RUNTIME_DIR, LOG_DIR } from '../config/pathConfig';
import { pathExists, remove } from 'fs-extra';
import {
    getIdByKfLocation,
    getProcessIdByKfLocation,
} from '../utils/busiUtils';
import { deleteProcess, startLedger, startMaster } from '../utils/processUtils';
import { Proc } from 'pm2';
import { watcher } from '../kungfu/watcher';

export const getAllKfConfigOriginData = (): Promise<
    Record<KfCategoryTypes, KfConfig[]>
> => {
    return getKfAllConfig().then((allConfig: KfConfigOrigin[]) => {
        const allConfigResolved = allConfig.map(
            (config: KfConfigOrigin): KfConfig => {
                return {
                    ...config,
                    category: KfCategoryEnum[
                        config.category
                    ] as KfCategoryTypes,
                    mode: KfModeEnum[config.mode] as KfModeTypes,
                };
            },
        );

        return {
            md: allConfigResolved.filter((config: KfConfig) => {
                return config.category === 'md';
            }),
            td: allConfigResolved.filter((config: KfConfig) => {
                return config.category === 'td';
            }),
            strategy: allConfigResolved.filter((config: KfConfig) => {
                return config.category === 'strategy';
            }),
            system: allConfigResolved.filter((config: KfConfig) => {
                return config.category === 'system';
            }),
        };
    });
};

export const deleteAllByKfLocation = (
    kfLocation: KfLocation,
): Promise<void> => {
    return removeKfConfig(kfLocation)
        .then(() => removeKfLocation(kfLocation))
        .then(() => removeLog(kfLocation));
};

function removeKfLocation(kfLocation: KfLocation): Promise<void> {
    const targetDir = path.resolve(
        KF_RUNTIME_DIR,
        kfLocation.category,
        kfLocation.group,
        kfLocation.name,
    );

    return pathExists(targetDir).then((isExisted: boolean) => {
        if (isExisted) {
            return remove(targetDir);
        }

        console.warn(`Location Dir ${targetDir} is not existed`);
    });
}

function removeLog(kfLocation: KfLocation): Promise<void> {
    const logPath = path.resolve(
        LOG_DIR,
        `${getProcessIdByKfLocation(kfLocation)}.log`,
    );
    return pathExists(logPath).then((isExisted: boolean) => {
        if (isExisted) {
            return remove(logPath);
        }

        console.warn(`Log Path ${logPath} is not existed`);
    });
}

export const switchKfLocation = (
    kfLocation: KfLocation,
    targetStatus: boolean,
): Promise<void | Proc> => {
    const processId = getProcessIdByKfLocation(kfLocation);

    if (!targetStatus) {
        if (kfLocation.category !== 'system') {
            if (!watcher || !watcher.isReadyToInteract(kfLocation)) {
                const processId = getProcessIdByKfLocation(kfLocation);
                throw new Error(`${processId} 还未准备就绪, 请稍后重试`);
            }
        }

        return deleteProcess(processId);
    }

    switch (kfLocation.category) {
        case 'system':
            if (kfLocation.name === 'master') {
                return startMaster(true);
            } else if (kfLocation.name === 'ledger') {
                return startLedger(true);
            }
        default:
            return Promise.resolve();
    }
};
