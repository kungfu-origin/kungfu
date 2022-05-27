import {
  getKfExtensionConfig,
  kfConfigItemsToProcessArgs,
} from '../utils/busiUtils';
import { Pm2ProcessStatusData, startTask } from '../utils/processUtils';
import path from 'path';
import { Proc } from 'pm2';
import { ensureRemoveLocation } from '.';

export const startTradingTask = (
  watcher: KungfuApi.Watcher,
  taskLocation: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
  soPath: string,
  args: string,
  configSettings: KungfuApi.KfConfigItem[],
) => {
  return ensureRemoveLocation(watcher, taskLocation, processStatusData).then(
    () => startTask(taskLocation, soPath, args, configSettings),
  );
};

export const startTradingTaskByCommand = async (
  watcher: KungfuApi.Watcher,
  taskLocation: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
  extKey: string,
  argsState: Record<string, KungfuApi.KfConfigValue>,
): Promise<Proc | void> => {
  const extConfigs = await getKfExtensionConfig();
  const extConfig: KungfuApi.KfExtConfig = (extConfigs['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    throw new Error(`${extKey} is not existed in extConfigs`);
  }

  const args: string = kfConfigItemsToProcessArgs(
    extConfig.settings || [],
    argsState,
  );

  const soPath = path.join(extConfig.extPath, extKey);
  return startTradingTask(
    watcher,
    taskLocation,
    processStatusData,
    soPath,
    args,
    extConfig.settings,
  );
};
