import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore, longfist } from '.';
import { kfLogger, getResultUntilValuable } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';

export const getAllKfRiskSettings = (): Promise<
  KungfuApi.RiskSettingOrigin[]
> => {
  kfLogger.info('Get kungfu RiskSettings');
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return getResultUntilValuable(() =>
      riskSettingStore.getAllRiskSetting(),
    ).then((riskSettings) => Object.values(riskSettings));
  } else {
    return Promise.resolve([]);
  }
};

export const setAllKfRiskSettings = (
  riskSettings: KungfuApi.RiskSettingForSave[],
): Promise<boolean> => {
  kfLogger.info('Set kungfu RiskSettings');
  const kfRiskSetting = longfist.RiskSetting();
  const riskSettingResolved = riskSettings
    .filter((item) => item.category === 'td' && item.group && item.name)
    .map((item) => ({
      ...kfRiskSetting,
      ...item,
    }));

  return getResultUntilValuable(() =>
    riskSettingStore.setAllRiskSetting(riskSettingResolved),
  );
};
