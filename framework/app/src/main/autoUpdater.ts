// import path from 'path';
// import os from 'os';
import { app, ipcMain, BrowserWindow } from 'electron';
import { autoUpdater } from 'electron-updater';
import {
  kfLogger,
  removeJournal,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import {
  downloadProcessUpdate,
  foundNewVersion,
  readyToStartAll,
  startDownloadNewVersion,
} from './events';
import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';

autoUpdater.logger = kfLogger;

function handleUpdateKungfu(MainWindows: BrowserWindow | null) {
  kfLogger.info('Kungfu client version', app.getVersion());
  // if (!app.isPackaged) return;

  const rootPackageJson = readRootPackageJsonSync();
  const updaterOption = rootPackageJson?.kungfuCraft?.autoUpdate?.update;

  if (!updaterOption) return;

  // 之后可以有更详细的目录划分
  // if (updaterOption.provider === 'generic') {
  //   updaterOption.url = path.resolve(
  //     updaterOption.url,
  //     `/update/${os.platform}/${app.getVersion()}`,
  //   );
  // }

  autoUpdater.autoDownload = false;
  autoUpdater.setFeedURL(updaterOption);

  autoUpdater.on('error', (error) => {
    kfLogger.error(
      error == null ? 'unknown' : (error.stack || error).toString(),
    );
  });

  autoUpdater.on('checking-for-update', () => {
    kfLogger.info('Checking for update');
  });

  autoUpdater.on('update-available', (info) => {
    kfLogger.info('Got a new kungfu client version', JSON.stringify(info));
    kfLogger.info('MainWindows', MainWindows);
    if (MainWindows) {
      foundNewVersion(MainWindows, info.version);

      ipcMain.once('auto-update-confirm-result', (_, result) => {
        kfLogger.info(result);
        if (result) {
          autoUpdater.downloadUpdate();
          startDownloadNewVersion(MainWindows);
        } else {
          readyToStartAll(MainWindows);
        }
      });
    }
  });

  autoUpdater.on('update-not-available', (info) => {
    kfLogger.info('Current version is up-to-date', JSON.stringify(info));
    MainWindows && readyToStartAll(MainWindows);
  });

  autoUpdater.on('update-downloaded', (info) => {
    kfLogger.info(JSON.stringify(info));
    MainWindows && downloadProcessUpdate(MainWindows, 100);

    setTimeout(() => {
      // app.exit();
      MainWindows?.destroy();
      removeJournal(KF_HOME).then(() => {
        autoUpdater.quitAndInstall(false, true);
      });
    }, 1000);
  });

  autoUpdater.on('download-progress', function (progressInfo) {
    kfLogger.info('Download progress: ', JSON.stringify(progressInfo));
    MainWindows && downloadProcessUpdate(MainWindows, progressInfo.percent);
  });

  ipcMain.on('auto-update-renderer-ready', () => {
    kfLogger.info('auto-update-renderer-ready');
    autoUpdater.checkForUpdates();
  });
}

export { handleUpdateKungfu };
