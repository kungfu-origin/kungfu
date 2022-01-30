import os from 'os';
import { app, BrowserWindow, dialog, nativeImage, shell } from 'electron';
import {
  reqRecordBeforeQuit,
  clearProcessBeforeQuitStart,
  clearProcessBeforeQuitEnd,
} from './events';
import path from 'path';
import {
  killKfc,
  killKungfu,
  killExtra,
  pm2KillGodDaemon,
  pm2Kill,
  deleteProcess,
  startTd,
  startMd,
  startStrategy,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  delayMilliSeconds,
  findTargetFromArray,
  getProcessIdByKfLocation,
  kfLogger,
  removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  KFC_DIR,
  KFC_PARENT_DIR,
  KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  getAllKfConfigOriginData,
  getScheduleTasks,
} from '@kungfu-trader/kungfu-js-api/actions';
import schedule from 'node-schedule';

import packageJSON from '@root/package.json';
declare const global: NodeJS.Global;

let BeforeQuitLoading = false;

export function openUrl(url: string): void {
  shell.openExternal(url);
}

export function showKungfuInfo(): void {
  const version = packageJSON.version;
  const electronVersion = packageJSON.devDependencies.electron;
  const info =
    `Version: ${version}\n` +
    `electron: ${electronVersion} \n` +
    `python: ${__python_version}\n` +
    `platform: ${os.platform()} \n` +
    `kungfu_home: ${KF_HOME} \n` +
    `kungfu_engine: ${KFC_DIR} \n` +
    `kungfu_resources: ${KFC_PARENT_DIR} \n` +
    `commit: ${__git_commit_version}` +
    `build_time: ${__build_timestamp}`;
  dialog.showMessageBox({
    type: 'info',
    message: 'Kungfu',
    defaultId: 0,
    detail: info,
    buttons: ['好的'],
    icon: nativeImage.createFromPath(
      path.join(global.__resources, 'logo', 'logo.png'),
    ),
  });
}

function KillAll(): Promise<void> {
  return new Promise((resolve) => {
    pm2Kill()
      .catch((err) => kfLogger.error(err.message))
      .finally(() => {
        pm2KillGodDaemon()
          .catch((err) => kfLogger.error(err.message))
          .finally(() => {
            killKfc()
              .catch((err) => kfLogger.error(err.message))
              .finally(() => {
                killKungfu()
                  .catch((err) => kfLogger.error(err.message))
                  .finally(() => {
                    killExtra()
                      .catch((err) => kfLogger.error(err.message))
                      .finally(() => {
                        resolve();
                      });
                  });
              });
          });
      });
  });
}

export function killAllBeforeQuit(mainWindow: BrowserWindow): Promise<void> {
  console.time('quit clean');
  clearProcessBeforeQuitStart(mainWindow);
  return KillAll().finally(() => {
    console.timeEnd('quit clean');
    clearProcessBeforeQuitEnd(mainWindow);
  });
}

export function showQuitMessageBox(
  mainWindow: BrowserWindow,
): Promise<boolean> {
  if (BeforeQuitLoading) {
    return Promise.reject(new Error('On Quitting Process'));
  }
  BeforeQuitLoading = true;
  return new Promise((resolve) => {
    dialog
      .showMessageBox({
        type: 'question',
        title: '提示',
        defaultId: 0,
        cancelId: 1,
        message: '退出应用会结束所有交易进程，确认退出吗？',
        buttons: ['确认', '取消'],
        icon: nativeImage.createFromPath(
          path.join(global.__resources, 'logo', 'logo.png'),
        ),
      })
      .then(({ response }) => {
        if (response === 0) {
          return Promise.all([
            reqRecordBeforeQuit(mainWindow),
            killAllBeforeQuit(mainWindow),
          ]).finally(() => {
            resolve(true);
            delayMilliSeconds(1000).then(() => {
              BeforeQuitLoading = false;
              app.quit();
            });
          });
        } else {
          BeforeQuitLoading = false;
          resolve(false);
        }
      });
  });
}

export function showCrashMessageBox(): Promise<boolean> {
  return dialog
    .showMessageBox({
      type: 'question',
      title: '提示',
      defaultId: 0,
      cancelId: 1,
      message: '功夫图形进程中断，该中断不会影响交易，是否重启图形进程？',
      buttons: ['确认', '取消'],
      icon: nativeImage.createFromPath(
        path.join(global.__resources, 'logo', 'logo.png'),
      ),
    })
    .then(({ response }) => {
      if (response === 0) {
        return true;
      } else {
        return false;
      }
    });
}

export const registerScheduleTasks = async (
  createWindowFunc: (
    reloadAfterCrashed: boolean,
    reloadBySchedule: boolean,
  ) => void,
): Promise<boolean> => {
  await schedule.gracefulShutdown();
  const scheduleTasks = await getScheduleTasks();
  const allKfConfig = await getAllKfConfigOriginData();
  const { td, md, strategy } = allKfConfig;

  const { active, tasks } = scheduleTasks;
  if (!active || !tasks) return;

  const tasksResolved = Object.values(
    scheduleTasks.tasks.reduce((avoidRepeatTasks, task) => {
      const id = `${task.processId}_${
        task.mode
      }_${+task.hour}_${+task.minute}_${+task.second}`;
      avoidRepeatTasks[id] = task;
      return avoidRepeatTasks;
    }, {} as Record<string, KungfuApi.ScheduleTask>),
  );

  tasksResolved
    .filter((item) => item.processId === 'core')
    .forEach((item) => {
      const rule = new schedule.RecurrenceRule();
      rule.hour = item.hour;
      rule.minute = item.minute;
      rule.second = item.second;

      schedule.scheduleJob(rule, () => {
        console.log('May the Force be with you -- Yoda');
        pm2Kill().finally(() => {
          pm2KillGodDaemon().finally(() => {
            kfLogger.info('Core restarted, pm2 killed all');
            removeJournal(KF_HOME);
            createWindowFunc(false, true);
          });
        });
      });
    });

  tasksResolved
    .filter((item) => item.processId.indexOf('td_') === 0)
    .forEach((item) => {
      const rule = new schedule.RecurrenceRule();
      rule.hour = item.hour;
      rule.minute = item.minute;
      rule.second = item.second;

      const tdProcessIds = td.map((item) => getProcessIdByKfLocation(item));
      if (!tdProcessIds.includes(item.processId)) {
        return;
      }

      const accountId = item.processId.toAccountId();
      if (item.mode === 'start') {
        schedule.scheduleJob(rule, () => {
          return startTd(accountId);
        });
      } else {
        schedule.scheduleJob(rule, () => {
          return deleteProcess(item.processId);
        });
      }
    });

  tasksResolved
    .filter((item) => item.processId.indexOf('md_') === 0)
    .forEach((item) => {
      const rule = new schedule.RecurrenceRule();
      rule.hour = item.hour;
      rule.minute = item.minute;
      rule.second = item.second;

      const mdProcessIds = md.map((item) => getProcessIdByKfLocation(item));
      if (!mdProcessIds.includes(item.processId)) {
        return;
      }

      const sourceName = item.processId.toSourceName();
      if (item.mode === 'start') {
        schedule.scheduleJob(rule, () => {
          startMd(sourceName);
        });
      } else {
        schedule.scheduleJob(rule, () => {
          deleteProcess(item.processId);
        });
      }
    });

  tasksResolved
    .filter((item) => item.processId.indexOf('strategy_') === 0)
    .forEach((item) => {
      const rule = new schedule.RecurrenceRule();
      rule.hour = item.hour;
      rule.minute = item.minute;
      rule.second = item.second;
      const strategyId = item.processId.toStrategyId();
      const targetStrategy: KungfuApi.KfConfig =
        findTargetFromArray<KungfuApi.KfConfig>(strategy, 'name', strategyId);

      if (!targetStrategy) {
        return;
      }

      const strategyPath =
        JSON.parse(targetStrategy?.value || '{}').strategy_path || '';

      if (item.mode === 'start') {
        schedule.scheduleJob(rule, () => {
          startStrategy(strategyId, strategyPath);
        });
      } else {
        schedule.scheduleJob(rule, () => {
          deleteProcess(item.processId);
        });
      }
    });

  return true;
};
