import { removeKfLocation, removeLog } from '.';
import { getKfExtensionConfig } from '../utils/busiUtils';
import {
  graceDeleteProcess,
  Pm2ProcessStatusData,
  startTask,
} from '../utils/processUtils';
import path from 'path';

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
  args: string,
) => {
  const extConfigs = await getKfExtensionConfig();
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    return Promise.reject(`${extKey} is not existed in extConfigs`);
  }

  const soPath = path.join(extConfig.extPath, extKey);
  return startTradingTask(
    watcher,
    taskLocation,
    processStatusData,
    soPath,
    args,
  );
};
