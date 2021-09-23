import electron from 'electron';
import { KF_HOME, KFC_PARENT_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { killGodDaemon, killKfc, killKungfu } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { platform } from '@kungfu-trader/kungfu-uicc/config/platformConfig';
import { killExtra } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { reqRecordBeforeQuit } from './events';

const path = require('path');
const { app, dialog } = electron;
const packageJSON = require('@kungfu-trader/kungfu-app/package.json');

export function openUrl(url) {
  electron.shell.openExternal(url);
}

export function showKungfuInfo() {
  const version = packageJSON.version;
  const electronVersion = packageJSON.devDependencies.electron;
  const info =
    `Version: ${version}\n` +
    `electron: ${electronVersion} \n` +
    `python version: ${python_version}\n` +
    `platform: ${platform} \n` +
    `kungfu_home: ${KF_HOME} \n` +
    `kungfu_engine: ${path.resolve(KFC_PARENT_DIR, 'kfc')} \n` +
    `kungfu_resources: ${path.resolve(KFC_PARENT_DIR)} \n` +
    `commit: ${git_commit_version}\n` +
    `build_tmp: ${build_timestamp}`;
    
  dialog.showMessageBox({
    type: 'info',
    message: 'Kungfu',
    defaultId: 0,
    detail: info,
    buttons: ['好的'],
    icon: path.join(__resources, 'logo', 'icon.png'),
  });
}

//结束所有进程
function KillAll() {
  return new Promise((resolve) => {
    killKfc()
      .catch((err) => console.error(err))
      .finally(() => {
        if (platform === 'linux') killKungfu();
        killGodDaemon()
          .catch((err) => console.error(err))
          .finally(() => {
            killExtra()
              .catch((err) => console.error(err))
              .finally(() => {
                resolve(true);
              });
          });
      });
  });
}

export function killAllBeforeQuit(mainWindow) {
  if (mainWindow && !mainWindow.isDestroyed()) mainWindow.hide();
  return KillAll();
}

//退出提示
export function showQuitMessageBox(mainWindow) {
    return dialog.showMessageBox(
      {
        type: 'question',
        title: '提示',
        defaultId: 0,
        cancelId: 1,
        message: '退出应用会结束所有交易进程，确认退出吗？',
        buttons: ['确认', '取消'],
        icon: path.join(__resources, 'logo', 'icon.png'),
      }
    ).then(({response}) => {
      console.log(index, '---')
      if (index === 0) {
        return reqRecordBeforeQuit(mainWindow)
          .then(() => {
            console.time('quit clean');
            return killAllBeforeQuit(mainWindow).finally(() => {
              console.timeEnd('quit clean');
              app.quit();
              return true;
            });
          });
      } else {
        return false
      }
    })
}
