import { configStore } from '@kungfu-trader/kungfu-js-api/kungfu';
import { KfConfig, KfLocation } from '@kungfu-trader/kungfu-js-api/typings';

import {
    kfLogger,
    hidePasswordByLogger,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

type AllConfig = Record<string, KfConfig>;

export const getKfAllConfig = (): Promise<KfConfig[]> => {
    kfLogger.info('Get ALL Kungfu Config');
    return Promise.resolve(
        Object.values(configStore.getAllConfig() as AllConfig),
    );
};
export const getKfConfigByName = (
    kfLocation: KfLocation,
): Promise<KfConfig> => {
    return Promise.resolve(
        configStore.getConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
        ),
    );
};

export const setKfConfig = (
    kfLocation: KfLocation,
    configValue: string,
): Promise<void> => {
    const configForLog = hidePasswordByLogger(configValue);
    kfLogger.info(
        `Set Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name} ${configForLog}`,
    );
    return Promise.resolve(
        configStore.setConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
            configValue,
        ),
    );
};

export const removeKfConfig = (kfLocation: KfLocation): Promise<void> => {
    kfLogger.info(
        `Remove Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name}`,
    );
    return Promise.resolve(
        configStore.removeConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
        ),
    );
};
