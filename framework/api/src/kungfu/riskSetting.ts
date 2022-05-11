import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore } from '.';
import { kfLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';

export const getAllKfRiskSettings = (): Promise<KungfuApi.RiskSetting[]> => {
  kfLogger.info('Get kungfu RiskControl');
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return Promise.resolve(Object.values(riskSettingStore.getAllRiskSetting()));
  } else {
    return Promise.resolve([]);
  }
};

export const setKfRiskSettings = (
  RiskSetting: KungfuApi.RiskSetting[],
): Promise<void> => {
  RiskSetting.filter((item) => {
    return item.account_id;
  }).map((item) => {
    const value: string = JSON.stringify(item);
    const KfLocation: KungfuApi.KfLocation = {
      category: 'td',
      group: item.account_id ? item.account_id.split('_')[0] : '',
      name: item.account_id || '',
      mode: 'live',
    };
    riskSettingStore.setRiskSetting(
      KfLocation.category,
      KfLocation.group,
      KfLocation.name,
      KfLocation.mode,
      value,
    );
  });
  return Promise.resolve();
};

export const RemoveRiskSetting = (
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
