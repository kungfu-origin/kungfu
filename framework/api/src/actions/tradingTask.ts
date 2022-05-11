import { removeKfLocation, removeLog } from '.';
import {
  getKfExtensionConfig,
  kfConfigItemsToProcessArgs,
} from '../utils/busiUtils';
import {
  graceDeleteProcess,
  Pm2ProcessStatusData,
  startTask,
} from '../utils/processUtils';
import path from 'path';
import { Proc } from 'pm2';

export const ensureRemoveTradingTask = (
  watcher: KungfuApi.Watcher,
  taskLocation: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
) => {
  return graceDeleteProcess(watcher, taskLocation, processStatusData)
    .then(() => removeKfLocation(taskLocation))
    .then(() => removeLog(taskLocation))
    .catch((err) => {
      console.error(err);
    });
};

export const startTradingTask = (
  watcher: KungfuApi.Watcher,
  taskLocation: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
  soPath: string,
  args: string,
) => {
  return ensureRemoveTradingTask(watcher, taskLocation, processStatusData).then(
    () => startTask(taskLocation, soPath, args),
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
  );
};
