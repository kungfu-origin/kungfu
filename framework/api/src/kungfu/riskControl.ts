import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore } from '../kungfu';
import { kfLogger, hidePasswordByLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';
import { getStrategyKfLocation } from './store';

type AllConfig = Record<string, KungfuApi.KfConfigOrigin>;

export const getAllKfRiskConfig = (): Promise<KungfuApi.KfConfigOrigin[]> => {
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    console.log(riskSettingStore.getAllRiskSetting());
    
    return Promise.resolve(
      Object.values(riskSettingStore.getAllRiskSetting() as AllConfig),
    );
  } else {
    return Promise.resolve([]);
  }
};

export const setKfRiskConfig = (
  kfLocation: KungfuApi.KfLocation,
  configValue: string,
): Promise<void> => {
  const configForLog = hidePasswordByLogger(configValue);
  kfLogger.info(
    `Set Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name} ${configForLog}`,
  );
  return Promise.resolve(
    riskSettingStore.setRiskSetting(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode,
      configValue,
    ),
  );
};

export const removeKfRiskConfig = (
  kfLocation: KungfuApi.KfLocation,
): Promise<void> => {
  kfLogger.info(
    `Remove Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name}`,
  );
  return Promise.resolve(
    riskSettingStore.removeRiskSetting(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode,
    ),
  );
};

export const getKfRiskConfig = (strategyId: string) => {
  const kfLocation: KungfuApi.KfLocation = getStrategyKfLocation(strategyId);
  return riskSettingStore.getRiskSetting(
    kfLocation.category,
    kfLocation.group,
    kfLocation.name,
    kfLocation.mode,
  );
};

export const getRiskControl = () => {
  kfLogger.info('Get kungfu Commission');
  return new Promise((resolve, reject) => {
    try {
      const riskConfigData = getAllKfRiskConfig();
      resolve(riskConfigData);
    } catch (err) {
      reject(err);
    }
  });
};