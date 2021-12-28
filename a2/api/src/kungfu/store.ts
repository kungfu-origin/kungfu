import { configStore } from '../kungfu';

import { kfLogger, hidePasswordByLogger } from '../utils/busiUtils';

type AllConfig = Record<string, KfConfigOrigin>;

export const getKfAllConfig = (): Promise<KfConfigOrigin[]> => {
    return Promise.resolve(
        Object.values(configStore.getAllConfig() as AllConfig),
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
