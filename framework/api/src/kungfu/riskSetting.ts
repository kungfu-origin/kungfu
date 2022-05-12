import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore, longfist } from '.';
import { kfLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';

export const getAllKfRiskSettings = (): Promise<
  KungfuApi.RiskSettingOrigin[]
> => {
  kfLogger.info('Get kungfu RiskSettings');
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return Promise.resolve(
      Object.values(riskSettingStore.getAllRiskSetting()).reverse(),
    );
  } else {
    return Promise.resolve([]);
  }
};

export const setAllKfRiskSettings = (
  riskSettings: KungfuApi.RiskSettingForSave[],
): Promise<boolean> => {
  kfLogger.info('Set kungfu RiskSettings');
  return new Promise((resolve, reject) => {
    const kfRiskSetting = longfist.RiskSetting();
    const riskSettingResolved = riskSettings
      .filter((item) => item.category === 'td' && item.group && item.name)
      .map((item) => ({
        ...kfRiskSetting,
        ...item,
      }));

    const result = riskSettingStore.setAllRiskSetting(riskSettingResolved);
    if (result) {
      resolve(result);
    } else {
      reject(result);
    }
  });
};
