import path from 'path';
import fse from 'fs-extra';
import { configStore } from '../kungfu';
import { kfLogger, hidePasswordByLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';
import { getKfConfig } from './strategy'

type AllConfig = Record<string, KungfuApi.KfConfigOrigin>;

export const getKfAllConfig = (): Promise<KungfuApi.KfConfigOrigin[]> => {
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return Promise.resolve(
      Object.values(configStore.getAllConfig() as AllConfig),
    );
  } else {
    return Promise.resolve([]);
  }
};

export const setKfConfig = (
  kfLocation: KungfuApi.KfLocation,
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

export const removeKfConfig = (
  kfLocation: KungfuApi.KfLocation,
): Promise<void> => {
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

export const getStrategyById = (
  strategyId: string,
): Promise<Array<Code.Strategy>> => {
  return new Promise((resolve) => {
    const strategyData: any = getKfConfig(strategyId);
    const strategy: Array<Code.Strategy> = [{ ...JSON.parse(strategyData.value || '{}') }];
    resolve(strategy);
  });
};

