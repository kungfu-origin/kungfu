import path from 'path';
import { getKfAllConfig, removeKfConfig } from '../kungfu/store';
import {
    KfCategoryEnum,
    KfCategoryTypes,
    KfModeEnum,
    KfModeTypes,
} from '../typings/enums';
import { KF_RUNTIME_DIR, LOG_DIR } from '../config/pathConfig';
import { pathExists, remove } from 'fs-extra';
import { getProcessIdByKfLocation } from '../utils/busiUtils';

export const getAllKfConfigOriginData = (): Promise<
    Record<KfCategoryTypes, KungfuApi.KfConfig[]>
> => {
    return getKfAllConfig().then((allConfig: KungfuApi.KfConfigOrigin[]) => {
        const allConfigResolved = allConfig.map(
            (config: KungfuApi.KfConfigOrigin): KungfuApi.KfConfig => {
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
            md: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
                return config.category === 'md';
            }),
            td: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
                return config.category === 'td';
            }),
            strategy: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
                return config.category === 'strategy';
            }),
            system: allConfigResolved.filter((config: KungfuApi.KfConfig) => {
                return config.category === 'system';
            }),
        };
    });
};

export const deleteAllByKfLocation = (
    kfLocation: KungfuApi.KfLocation,
): Promise<void> => {
    return removeKfConfig(kfLocation)
        .then(() => removeKfLocation(kfLocation))
        .then(() => removeLog(kfLocation));
};

function removeKfLocation(kfLocation: KungfuApi.KfLocation): Promise<void> {
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

function removeLog(kfLocation: KungfuApi.KfLocation): Promise<void> {
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
