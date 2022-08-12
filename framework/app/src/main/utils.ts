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
  pm2Kill,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  delayMilliSeconds,
  deleteNNFiles,
  kfLogger,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  KFC_DIR,
  KFC_PARENT_DIR,
  KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import packageJSON from '@kungfu-trader/kungfu-app/package.json';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

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
      path.join(globalThis.__publicResources, 'logo', 'logo.png'),
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
                    deleteNNFiles()
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
        title: t('prompt'),
        defaultId: 0,
        cancelId: 1,
        message: t('quit_confirm'),
        buttons: [t('confirm'), t('cancel')],
        icon: nativeImage.createFromPath(
          path.join(globalThis.__publicResources, 'logo', 'logo.png'),
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
        path.join(globalThis.__publicResources, 'logo', 'logo.png'),
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
