import {
    getKfAllConfig,
    getKfConfigByName,
    setKfConfig,
    removeKfConfig,
} from '@kungfu-trader/kungfu-js-api/kungfu/store';
import {
    KfConfig,
    KfCategoryEnum,
    KfCategoryTypes,
    StrategyData,
} from '@kungfu-trader/kungfu-js-api/typings';
import {
    logger,
    hidePasswordByLogger,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

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
    strategyId: string,
): Promise<StrategyData> => {
    return getKfConfigByName(strategyId, 'strategy').then(
        (strategyConfig: KfConfig) => {
            return {
                ...JSON.parse(strategyConfig.value || '{}'),
            } as StrategyData;
        },
    );
};

export const addTd = (accountId: string, config: string): Promise<void> => {
    return new Promise((resolve) => {
        setKfConfig(accountId, 'td', config);
        logger.info(
            'Add Trade Account',
            accountId,
            hidePasswordByLogger(config),
        );
        resolve();
    });
};

export const addMd = (sourceId: string, config: string): Promise<void> => {
    return new Promise((resolve) => {
        setKfConfig(sourceId, 'md', config);
        logger.info(
            'Add Market Source',
            sourceId,
            hidePasswordByLogger(config),
        );
        resolve();
    });
};

export const addStrategy = (
    strategyId: string,
    strategyPath: string,
): Promise<void> => {
    const addTime = +new Date().getTime() * Math.pow(10, 6);
    return new Promise((resolve) => {
        setKfConfig(
            strategyId,
            'strategy',
            JSON.stringify({
                strategy_id: strategyId,
                strategy_path: strategyPath,
                add_time: addTime,
            } as StrategyData),
        );
        logger.info('Add Strategy', strategyId, strategyPath);
        resolve();
    });
};

export const deleteTd = (accountId: string): Promise<void> => {
    return new Promise((resolve) => {
        removeKfConfig(accountId, 'td');
        logger.info('Delete Trade Account', accountId);
        resolve();
    });
};

export const deleteMd = (sourceId: string): Promise<void> => {
    return new Promise((resolve) => {
        removeKfConfig(sourceId, 'md');
        logger.info('Delete Market Source', sourceId);
        resolve();
    });
};

export const deleteStrategy = (strategyId: string): Promise<void> => {
    return new Promise((resolve) => {
        removeKfConfig(strategyId, 'strategy');
        logger.info('Delete Strategy', strategyId);
        resolve();
    });
};
