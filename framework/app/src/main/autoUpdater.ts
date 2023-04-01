import semver from 'semver';
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
import { RootConfigJSON } from '@kungfu-trader/kungfu-js-api/typings/global';

autoUpdater.logger = kfLogger;

const getChannel = (isPrerelease: boolean) => {
  const prefix = 'kungfu-update';
  const baseChannel = isPrerelease ? 'alpha' : 'latest';
  return `${prefix}-${baseChannel}`;
};

const getProjectName = (rootConfigJson: RootConfigJSON): string => {
  if (rootConfigJson.name) {
    const names = rootConfigJson.name.split('/');
    if (names.length) {
      return names[1];
    } else {
      return rootConfigJson.name;
    }
  }

  return `kungfu-project-name-undefined-${new Date().toDateString()}`;
};

function handleUpdateKungfu(MainWindow: BrowserWindow | null) {
  kfLogger.info('Kungfu client version: ', app.getVersion());
  kfLogger.info('Kungfu client isPacked: ', app.isPackaged);
  if (!app.isPackaged) return;

  const rootPackageJson = readRootPackageJsonSync();
  const updaterOption = rootPackageJson?.kungfuCraft?.autoUpdate?.update;

  if (!rootPackageJson || !updaterOption) return;

  const projectName = getProjectName(rootPackageJson);
  const version = semver.parse(
    rootPackageJson.version as string,
  ) as semver.SemVer;
  const targetVersion = version.prerelease.length
    ? semver.inc(version, 'prerelease', 'alpha')
    : semver.inc(version, 'patch');
  const artifactPath = `${projectName}/v${version.major}/v${targetVersion}`;

  kfLogger.info('Kungfu autoUpdater artifact path: ', artifactPath);

  updaterOption.channel = getChannel(!!version.prerelease.length);

  if (updaterOption.provider === 'generic') {
    updaterOption.url = `${updaterOption.url}/${artifactPath}`;
  } else if (updaterOption.provider === 's3') {
    updaterOption.path = artifactPath;
  }

  kfLogger.info('Kungfu autoUpdater option: ', JSON.stringify(updaterOption));

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
    if (MainWindow) {
      foundNewVersion(MainWindow, info.version);

      ipcMain.on('auto-update-confirm-result', (_, result) => {
        if (result) {
          autoUpdater.downloadUpdate();
          startDownloadNewVersion(MainWindow);
        }
      });

      ipcMain.on('auto-update-to-start-download', () => {
        autoUpdater.downloadUpdate();
        startDownloadNewVersion(MainWindow);
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
