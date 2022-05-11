import path from 'path';
import fse from 'fs-extra';
import { riskSettingStore } from '.';
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
      account_id,
      source_id,
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
      account_id,
      source_id,
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
  riskConfigResolved.forEach((item) => {
    const value: string = JSON.stringify(item);
    riskSettingStore.setRiskSetting(
      item.category,
      item.group,
      item.name,
      item.mode,
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

export const getKfRiskConfig = (
  strategyId: string,
): Promise<KungfuApi.RiskSetting> => {
  const kfLocation: KungfuApi.KfLocation = getStrategyKfLocation(strategyId);
  return Promise.resolve(
    riskSettingStore.getRiskSetting(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode,
    ),
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
