// import path from 'path';
// import os from 'os';
import { app, ipcMain, BrowserWindow } from 'electron';
import { autoUpdater } from 'electron-updater';
import {
  delayMilliSeconds,
  kfLogger,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import {
  downloadProcessUpdate,
  foundNewVersion,
  reqRecordBeforeQuit,
  sendUpdatingError,
  startDownloadNewVersion,
  updateNotAvailable,
} from './events';
import { KF_HOME } from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import { killAllBeforeQuit } from './utils';
import { removeTargetFilesInFolder } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';

autoUpdater.logger = kfLogger;

function handleUpdateKungfu(MainWindow: BrowserWindow | null) {
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
  autoUpdater.autoInstallOnAppQuit = false;
  autoUpdater.autoRunAppAfterInstall = true;
  autoUpdater.setFeedURL(updaterOption);

  autoUpdater.on('error', (error) => {
    kfLogger.error(
      error == null ? 'unknown' : (error.stack || error).toString(),
    );
    MainWindow && sendUpdatingError(MainWindow, error);
  });

  autoUpdater.on('checking-for-update', () => {
    kfLogger.info('Checking for update');
  });

  autoUpdater.on('update-available', (info) => {
    kfLogger.info('Got a new kungfu client version', JSON.stringify(info));
    kfLogger.info('MainWindow', MainWindow);
    if (MainWindow) {
      foundNewVersion(MainWindow, info.version);

      ipcMain.once('auto-update-confirm-result', (_, result) => {
        kfLogger.info(result);
        if (result) {
          autoUpdater.downloadUpdate();
          startDownloadNewVersion(MainWindow);
        } else {
          ipcMain.on('auto-update-to-start-download', () => {
            autoUpdater.downloadUpdate();
            startDownloadNewVersion(MainWindow);
          });
        }
      });
    }
  });

  autoUpdater.on('update-not-available', (info) => {
    kfLogger.info('Current version is up-to-date', JSON.stringify(info));
    MainWindow && updateNotAvailable(MainWindow);
  });

  autoUpdater.on('update-downloaded', (info) => {
    kfLogger.info('update-downloaded', JSON.stringify(info));
    MainWindow && downloadProcessUpdate(MainWindow, 100);

    ipcMain.on('auto-update-quit-and-install', () => {
      if (!MainWindow) return;

      Promise.all([
        reqRecordBeforeQuit(MainWindow),
        killAllBeforeQuit(MainWindow),
      ]).finally(() => {
        delayMilliSeconds(1000).then(() => {
          removeTargetFilesInFolder(
            KF_HOME,
            ['.db', '.journal'],
            ['etc', 'config.db'],
          ).then(() => {
            autoUpdater.quitAndInstall(false, true);
            // MainWindow.destroy();
            app.exit();
          });
        });
      });
    });
  });

  autoUpdater.on('download-progress', function (progressInfo) {
    kfLogger.info('Download progress: ', JSON.stringify(progressInfo));
    MainWindow && downloadProcessUpdate(MainWindow, progressInfo.percent);
  });

  ipcMain.on('auto-update-renderer-ready', () => {
    kfLogger.info('auto-update-renderer-ready');
    autoUpdater.checkForUpdates();
  });
}

export { handleUpdateKungfu };
