import path from 'path';
import {
    getKfAllConfig,
    getKfConfigByName,
    removeKfConfig,
} from '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
    KfConfig,
    KfCategoryEnum,
    KfCategoryTypes,
    StrategyData,
    KfLocation,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    KF_RUNTIME_DIR,
    LOG_DIR,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { pathExists, remove } from 'fs-extra';
import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/kungfu/utils';

export const getTdList = (): Promise<KfConfig[]> => {
    return getKfAllConfig().then((allConfig: KfConfig[]) => {
        return allConfig.filter((config: KfConfig) => {
            return config.category === KfCategoryEnum.td;
        });
    });
};

export const getMdList = (): Promise<KfConfig[]> => {
    return getKfAllConfig().then((allConfig: KfConfig[]) => {
        return allConfig.filter((config: KfConfig) => {
            return config.category === KfCategoryEnum.md;
        });
    });
};

export const getStrategyList = (): Promise<KfConfig[]> => {
    return getKfAllConfig().then((allConfig: KfConfig[]) => {
        return allConfig.filter((config: KfConfig) => {
            return config.category === KfCategoryEnum.strategy;
        });
    });
};

export const getAllKfConfigList = (): Promise<
    Record<KfCategoryTypes, KfConfig[]>
> => {
    return getKfAllConfig().then((allConfig: KfConfig[]) => {
        return {
            md: allConfig.filter((config: KfConfig) => {
                return config.category === KfCategoryEnum.md;
            }),
            td: allConfig.filter((config: KfConfig) => {
                return config.category === KfCategoryEnum.td;
            }),
            strategy: allConfig.filter((config: KfConfig) => {
                return config.category === KfCategoryEnum.strategy;
            }),
            system: allConfig.filter((config: KfConfig) => {
                return config.category === KfCategoryEnum.system;
            }),
        };
    });
};

export const getStrategyDataById = (
    kfLocation: KfLocation,
): Promise<StrategyData> => {
    return getKfConfigByName(kfLocation).then((strategyConfig: KfConfig) => {
        return {
            ...JSON.parse(strategyConfig.value || '{}'),
        } as StrategyData;
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
