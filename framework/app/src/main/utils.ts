import os from 'os';
import fse from 'fs-extra';
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
  BASE_DB_DIR,
  KFC_DIR,
  KFC_PARENT_DIR,
  KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  getAllKfConfigOriginData,
  getScheduleTasks,
} from '@kungfu-trader/kungfu-js-api/actions';
import schedule from 'node-schedule';

import packageJSON from '@kungfu-trader/kungfu-app/package.json';

import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

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
    ' \n' +
    `kungfu_home: ${KF_HOME} \n` +
    ' \n' +
    `kungfu_engine: ${KFC_DIR} \n` +
    ' \n' +
    `kungfu_resources: ${KFC_PARENT_DIR} \n` +
    ' \n' +
    `commit: ${__git_commit_version} \n` +
    `build_time: ${__build_timestamp}`;
  dialog.showMessageBox({
    type: 'info',
    message: 'Kungfu',
    defaultId: 0,
    detail: info,
    buttons: [t('ok')],
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
        title: t('prompt'),
        defaultId: 0,
        cancelId: 1,
        message: t('quit_confirm'),
        buttons: [t('confirm'), t('cancel')],
        icon: nativeImage.createFromPath(
          path.join(global.__resources, 'logo', 'logo.png'),
        ),
      })
      .then(({ response }) => {
        if (response === 0) {
          Promise.all([
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
      title: t('prompt'),
      defaultId: 0,
      cancelId: 1,
      message: t('restart_process'),
      buttons: [t('confirm'), t('cancel')],
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
  if (!fse.pathExistsSync(path.join(BASE_DB_DIR, 'config.db'))) {
    return false;
  }

  await schedule.gracefulShutdown();
  const scheduleTasks = await getScheduleTasks();
  const allKfConfig = await getAllKfConfigOriginData();
  const { td, md, strategy } = allKfConfig;

  const { active, tasks } = scheduleTasks;
  if (!active || !tasks) return false;

  const tasksResolved: KungfuApi.ScheduleTask[] = Object.values(
    (scheduleTasks.tasks || []).reduce((avoidRepeatTasks, task) => {
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
          kfLogger.info('Core restarted, pm2 killed all');
          removeJournal(KF_HOME);
          createWindowFunc(false, true);
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
      const targetStrategy: KungfuApi.KfConfig | null =
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
