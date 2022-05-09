// import path from 'path';
// import fse from 'fs-extra';
// import { riskSettingStore } from '../kungfu';
// import { kfLogger, hidePasswordByLogger } from '../utils/busiUtils';
// import { BASE_DB_DIR } from '../config/pathConfig';
// import { getStrategyKfLocation } from './store';

// type AllConfig = Record<string, KungfuApi.KfConfigOrigin>;

// export const getAllRiskConfig = (): Promise<KungfuApi.KfConfigOrigin[]> => {
//   if (fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
//     return Promise.resolve(
//       Object.values(riskSettingStore.GetAllRiskSetting() as AllConfig),
//     );
//   } else {
//     return Promise.resolve([]);
//   }
// };

// export const setRiskConfig = (
//   kfLocation: KungfuApi.KfLocation,
//   configValue: string,
// ): Promise<void> => {
//   const configForLog = hidePasswordByLogger(configValue);
//   kfLogger.info(
//     `Set Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name} ${configForLog}`,
//   );
//   return Promise.resolve(
//     riskSettingStore.SetRiskSetting(
//       kfLocation.category,
//       kfLocation.group,
//       kfLocation.name,
//       kfLocation.mode,
//       configValue,
//     ),
//   );
// };

// export const removeRiskConfig = (
//   kfLocation: KungfuApi.KfLocation,
// ): Promise<void> => {
//   kfLogger.info(
//     `Remove Kungfu Config ${kfLocation.category} ${kfLocation.group} ${kfLocation.name}`,
//   );
//   return Promise.resolve(
//     riskSettingStore.RemoveRiskSetting(
//       kfLocation.category,
//       kfLocation.group,
//       kfLocation.name,
//       kfLocation.mode,
//     ),
//   );
// };

// export const getRiskConfig = (strategyId: string) => {
//   const kfLocation: KungfuApi.KfLocation = getStrategyKfLocation(strategyId);
//   return riskSettingStore.GetRiskSetting(
//     kfLocation.category,
//     kfLocation.group,
//     kfLocation.name,
//     kfLocation.mode,
//   );
// };
