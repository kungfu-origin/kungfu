import { app, dialog } from 'electron';
import { autoUpdater } from 'electron-updater';
import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';

autoUpdater.logger = kfLogger;

function handleUpdateKungfu() {
  kfLogger.info('Kungfu client version', app.getVersion());
  if (!app.isPackaged) return;

  const rootPackageJson = readRootPackageJsonSync();
  const updaterOption = rootPackageJson?.kungfuAutoUpdate?.update;

  // const server = 'https://your-deployment-url.com';
  // const url = `${server}/update/${process.platform}/${app.getVersion()}`;

  if (!updaterOption) return;

  autoUpdater.autoDownload = false;
  autoUpdater.setFeedURL(updaterOption);

  autoUpdater.on('error', (error) => {
    dialog.showErrorBox(
      'Error: ',
      error == null ? 'unknown' : (error.stack || error).toString(),
    );
  });

  autoUpdater.on('checking-for-update', () => {
    kfLogger.info('Checking for update');
  });

  autoUpdater.on('update-available', (info) => {
    kfLogger.info(
      'Got a new kungfu client version, will auto download it',
      JSON.stringify(info),
    );
    autoUpdater.downloadUpdate();
  });

  autoUpdater.on('update-not-available', (info) => {
    kfLogger.info('Current version is up-to-date', JSON.stringify(info));
  });

  autoUpdater.on('update-downloaded', (info) => {
    kfLogger.info(JSON.stringify(info));
    dialog
      .showMessageBox({
        type: 'info',
        title: '软件升级',
        message: '发现新版本，是否立即升级？',
        buttons: ['是的', '稍后'],
      })
      .then((resp) => {
        if (resp.response === 0) {
          kfLogger.info('Begin to install new version ...');
          autoUpdater.quitAndInstall(false, true);
        }
      });
  });

  autoUpdater.on('download-progress', function (progressObj) {
    kfLogger.info('Download progress: ', JSON.stringify(progressObj));
  });

  autoUpdater.checkForUpdates();
}

export { handleUpdateKungfu };
