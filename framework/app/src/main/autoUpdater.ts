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
let isRendererReady = false;

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

const getAlphaReleaseVersion = (version: semver.SemVer) => {
  return `${version.major}.${version.minor}.${version.patch}`;
};

const getNextMinorReleaseVersion = (version: semver.SemVer) => {
  return `${version.major}.${version.minor + 1}.0`;
};

function handleUpdateKungfu(
  MainWindow: BrowserWindow | null,
  targetVersions: string[] = [],
) {
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

  if (!targetVersions.length) {
    const isPrerelease = !!version.prerelease.length;
    if (isPrerelease) {
      targetVersions.push(
        semver.inc(version, 'prerelease', 'alpha') || 'kungfu-version-unknow',
        getAlphaReleaseVersion(version),
      );
    } else {
      targetVersions.push(
        semver.inc(version, 'patch') || 'kungfu-version-unknow',
        getNextMinorReleaseVersion(version),
      );
    }
  }

  if (!targetVersions.length) return;

  kfLogger.info(
    'Kungfu autoUpdater all target versions: ',
    JSON.stringify(targetVersions),
  );

  const curTargetVersion = targetVersions.shift();
  if (!curTargetVersion) return;

  const artifactPath = `${projectName}/v${version.major}/v${curTargetVersion}`;

  kfLogger.info('Kungfu autoUpdater artifact path: ', artifactPath);

  updaterOption.channel = getChannel(curTargetVersion.includes('-alpha'));

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
  autoUpdater.removeAllListeners();

  let curErrorCalledNext = false;
  autoUpdater.on('error', (error) => {
    kfLogger.error('Kungfu autoUpdater error message: ', error?.message);
    MainWindow && sendUpdatingError(MainWindow, error);

    if (!curErrorCalledNext && targetVersions.length) {
      curErrorCalledNext = true;
      handleUpdateKungfu(MainWindow, targetVersions);
    }
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
    if (targetVersions.length) handleUpdateKungfu(MainWindow, targetVersions);
  });

  autoUpdater.on('update-downloaded', (info) => {
    kfLogger.info('update-downloaded', JSON.stringify(info));
    MainWindow && downloadProcessUpdate(MainWindow, 100);

    ipcMain.on('auto-update-quit-and-install', () => {
      if (!MainWindow) return;

      Promise.all([
        reqRecordBeforeQuit(MainWindow),
        killAllBeforeQuit(MainWindow),
      ])
        .catch((err) => {
          kfLogger.error(err);
        })
        .finally(() => {
          delayMilliSeconds(1000).then(() => {
            removeTargetFilesInFolder(
              KF_HOME,
              ['.db', '.journal'],
              ['etc', 'config.db'],
            ).then((results) => {
              MainWindow.webContents
                .executeJavaScript(
                  `localStorage.setItem('needClearJournal', '1');`,
                  true,
                )
                .catch((err) => {
                  kfLogger.error(err);
                })
                .finally(() => {
                  results.errors.forEach((error) => kfLogger.error(error));
                  delayMilliSeconds(1000).then(() => {
                    autoUpdater.quitAndInstall(false, true);
                    app.exit();
                  });
                });
            });
          });
        });
    });
  });

  autoUpdater.on('download-progress', function (progressInfo) {
    kfLogger.info('Download progress: ', JSON.stringify(progressInfo));
    MainWindow && downloadProcessUpdate(MainWindow, progressInfo.percent);
  });

  if (isRendererReady) {
    autoUpdater.checkForUpdates();
  } else {
    ipcMain.on('auto-update-renderer-ready', () => {
      kfLogger.info('auto-update-renderer-ready');
      autoUpdater.checkForUpdates();
      isRendererReady = true;
    });
  }
}

export { handleUpdateKungfu };
