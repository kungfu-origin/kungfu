import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore } from '../kungfu';
import { kfLogger } from '../utils/busiUtils';
import { BASE_DB_DIR } from '../config/pathConfig';
import { getStrategyKfLocation } from './store';

export const getAllKfRiskConfig = (): Promise<KungfuApi.RiskSetting[]> => {
  if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return Promise.resolve(Object.values(riskSettingStore.getAllRiskSetting()));
  } else {
    return Promise.resolve([]);
  }
};

export const setKfRiskConfig = (
  RiskSetting: KungfuApi.RiskSetting[],
): Promise<void> => {
  const riskConfigResolved = RiskSetting.map((item) => {
    const {
      category,
      group,
      name,
      mode,
      max_order_volume,
      max_daily_volume,
      white_list,
      self_filled_check,
      max_cancel_ratio,
    } = item;
    return {
      category,
      group,
      name,
      mode,
      max_order_volume,
      max_daily_volume,
      white_list,
      self_filled_check,
      max_cancel_ratio,
    };
  });
  const value: string = JSON.stringify(riskConfigResolved);
  riskSettingStore.setRiskSetting({ value: value });
  return Promise.resolve();
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

export const getRiskControl = (): Promise<KungfuApi.RiskSetting[]> => {
  kfLogger.info('Get kungfu RiskControl');
  return new Promise(async (resolve, reject) => {
    try {
      const riskConfigData = await getAllKfRiskConfig();
      resolve(riskConfigData);
    } catch (err) {
      reject(err);
    }
  });
};
