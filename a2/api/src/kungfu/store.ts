// import { global.configStore } from '@kungfu-trader/kungfu-js-api/kungfu';
import { toKfLocation } from '@kungfu-trader/kungfu-js-api/kungfu/utils';
import {
    KfCategoryTypes,
    KfConfig,
    KfLocation,
} from '@kungfu-trader/kungfu-js-api/typings';
import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

type AllConfig = Record<string, KfConfig>;

export const getKfAllConfig = (): Promise<KfConfig[]> => {
    kfLogger.info('Get ALL Kungfu Config');
    return Promise.resolve(
        Object.values(global.configStore.getAllConfig() as AllConfig),
    );
};
export const getKfConfigByName = (
    name: string,
    type: KfCategoryTypes,
): Promise<KfConfig> => {
    kfLogger.info('Get Kungfu Config By Name', name, type);
    const kfLocation: KfLocation = toKfLocation(name, type);
    return Promise.resolve(
        global.configStore.getConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
        ),
    );
};

export const setKfConfig = (
    name: string,
    type: KfCategoryTypes,
    configValue: string,
): Promise<void> => {
    kfLogger.info('Set Kungfu Config', name, type, configValue);
    const kfLocation: KfLocation = toKfLocation(name, type);
    return Promise.resolve(
        global.configStore.setConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
            configValue,
        ),
    );
};

export const removeKfConfig = (
    name: string,
    type: KfCategoryTypes,
): Promise<void> => {
    kfLogger.info('Remove Kungfu Config', name, type);
    const kfLocation = toKfLocation(name, type);
    return Promise.resolve(
        global.configStore.removeConfig(
            kfLocation.category,
            kfLocation.group,
            kfLocation.name,
            kfLocation.mode,
        ),
    );
};
