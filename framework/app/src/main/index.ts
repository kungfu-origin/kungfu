import {
  BrowserWindow,
  screen,
  app,
  globalShortcut,
  Menu,
  MenuItemConstructorOptions,
  shell,
  ipcMain,
} from 'electron';
import * as remoteMain from '@electron/remote/main';
import path from 'path';
import os from 'os';
import {
  showQuitMessageBox,
  showCrashMessageBox,
  showKungfuInfo,
  openUrl,
  registerScheduleTasks,
} from '@kungfu-trader/kungfu-app/src/main/utils';
import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { killExtra } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  clearDB,
  clearJournal,
  exportAllTradingData,
  openLogFile,
  openSettingDialog,
  resetMainDashboard,
} from './events';

import {
  BASE_DB_DIR,
  KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';
import {
  initKfConfig,
  initKfDefaultInstruments,
  ensureKungfuKey,
} from '@kungfu-trader/kungfu-js-api/config';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

let MainWindow: BrowserWindow | null = null;
let AllowQuit = false;
let CrashedReloading = false;
let SecheduleReloading = false;
const isDev = process.env.NODE_ENV === 'development';
const isMac = os.platform() === 'darwin';

remoteMain.initialize();
setMenu();
initKfConfig();
initKfDefaultInstruments();
ensureKungfuKey();

function createWindow(reloadAfterCrashed = false, reloadBySchedule = false) {
  if (reloadAfterCrashed) {
    MainWindow && MainWindow.destroy();
    CrashedReloading = true;
  }

  if (reloadBySchedule) {
    MainWindow && MainWindow.destroy();
    SecheduleReloading = true;
  }

  const { width, height } = screen.getPrimaryDisplay().size;
  MainWindow = new BrowserWindow({
    show: false,
    width: width > 1920 ? 1920 : width,
    height: height > 1200 ? 1200 : height,
    useContentSize: true,
    webPreferences: {
      nodeIntegration: true,
      nodeIntegrationInWorker: true,
      contextIsolation: false,
      enableRemoteModule: true,
      additionalArguments: [
        reloadAfterCrashed ? 'reloadAfterCrashed' : '',
        reloadBySchedule ? 'reloadBySchedule' : '',
      ],
    },
    backgroundColor: '#000',
  });

  if (isDev) {
    MainWindow.loadURL('http://localhost:9090/index.html');
  } else {
    const filePath = path.join(__dirname, 'index.html');
    MainWindow.loadFile(filePath);
  }

  MainWindow.on('ready-to-show', function () {
    MainWindow && MainWindow.show();
    MainWindow && MainWindow.focus();
    if (reloadAfterCrashed) {
      CrashedReloading = false;
    }

    if (reloadBySchedule) {
      SecheduleReloading = false;
    }
  });

  MainWindow.on('close', (e) => {
    if (CrashedReloading || SecheduleReloading) {
      MainWindow && MainWindow.destroy();
      return;
    }

    if (!AllowQuit) {
      e.preventDefault();
      if (MainWindow) {
        showQuitMessageBox(MainWindow)
          .then((res) => {
            if (res) {
              AllowQuit = true;
            }
          })
          .catch((err) => {
            console.error(err);
          });
      }
    } else {
      MainWindow && MainWindow.destroy();
      return;
    }
  });

  MainWindow.webContents.on('render-process-gone', (_event, details) => {
    kfLogger.error(
      '[MainWindow.webContents] crashed' + new Date(),
      JSON.stringify(details),
    );
    if (AllowQuit) return;
    showCrashMessageBox().then((confirm) => {
      if (!confirm) return;
      createWindow(true);
    });
  });

  MainWindow.webContents.on('unresponsive', () => {
    kfLogger.error('[MainWindow.webContents] unresponsive' + new Date());
    if (AllowQuit) return;
    showCrashMessageBox().then((confirm) => {
      if (!confirm) return;
      createWindow(true);
    });
  });
}

// 防止重开逻辑
if (process.env.NODE_ENV !== 'development') {
  const gotTheLock = app.requestSingleInstanceLock();
  if (!gotTheLock) {
    AllowQuit = true;
    app.quit();
  } else {
    app.on('second-instance', () => {
      console.log(' ======== Second-Instance ========');
      if (MainWindow) {
        if (MainWindow.isMinimized()) MainWindow.restore();
        MainWindow.focus();
      }
    });
  }
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
let appReady = false,
  killExtraFinished = false;
app.on('ready', () => {
  appReady = true;
  if (appReady && killExtraFinished) createWindow();

  globalShortcut.register('CommandOrControl+Shift+I', () => {
    const focusedWin = BrowserWindow.getFocusedWindow();
    if (focusedWin) {
      focusedWin.webContents.openDevTools();
    }
  });
});

//一上来先把所有之前意外没关掉的 pm2/kfc 进程kill掉
console.time('init clean');
killExtra()
  .catch((err) => kfLogger.error(err.message))
  .finally(() => {
    console.timeEnd('init clean');
    killExtraFinished = true;
    if (appReady && killExtraFinished) createWindow();
  });

// Quit when all windows are closed.
app.on('window-all-closed', function () {
  // On macOS it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  app.exit();
});

app.on('activate', function () {
  // On macOS it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (MainWindow && MainWindow.isDestroyed()) createWindow();
  else if (MainWindow && MainWindow.isVisible()) MainWindow.focus();
  else MainWindow && MainWindow.show();
});

app.on('will-quit', (e) => {
  if (AllowQuit) {
    globalShortcut.unregisterAll();
    return;
  }

  e.preventDefault();
});

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.

function setMenu() {
  //添加快捷键
  const applicationOptions = [
    { label: t('about_kungfu'), click: () => showKungfuInfo() },
    {
      label: t('set'),
      accelerator: 'CommandOrControl+,',
      click: () => MainWindow && openSettingDialog(MainWindow),
    },
    {
      label: t('close'),
      accelerator: 'CommandOrControl+W',
      click: () => {
        const focusedWin = BrowserWindow.getFocusedWindow();
        if (focusedWin) {
          focusedWin.close();
        }
      },
    },
  ];

  if (isMac) {
    applicationOptions.push({
      label: t('quit'),
      accelerator: 'Command+Q',
      click: () => app.quit(),
    });
  }

  const template: MenuItemConstructorOptions[] = [
    {
      label: t('KungFu'),
      submenu: applicationOptions,
    },
    {
      //此处必要, 不然electron内使用复制粘贴会无效
      label: t('edit'),
      submenu: [
        {
          label: t('copy'),
          accelerator: 'CommandOrControl+C',
          role: 'copy',
        },
        {
          label: t('paste'),
          accelerator: 'CommandOrControl+V',
          role: 'paste',
        },
        {
          label: t('select_all'),
          accelerator: 'CommandOrControl+A',
          role: 'selectAll',
        },
        {
          label: t('undo'),
          accelerator: 'CommandOrControl+Z',
          role: 'undo',
        },
      ],
    },
    {
      label: t('file'),
      submenu: [
        {
          label: t('open_resources_directory'),
          click: () => shell.showItemInFolder(KF_HOME),
        },
        {
          label: t('open_install_directory'),
          click: () => shell.showItemInFolder(app.getAppPath()),
        },
        {
          label: t('open_basic_configuration'),
          click: () =>
            shell.showItemInFolder(path.join(BASE_DB_DIR, 'config.db')),
        },
        {
          label: t('browsing_log'),
          accelerator: 'CommandOrControl+L',
          click: () => MainWindow && openLogFile(MainWindow),
        },
      ],
    },
    {
      label: t('run'),
      submenu: [
        {
          label: t('clear_journal'),
          click: () => MainWindow && clearJournal(MainWindow),
        },
        {
          label: t('clear_DB'),
          click: () => MainWindow && clearDB(MainWindow),
        },
        {
          label: t('reset_main_panel'),
          click: () => MainWindow && resetMainDashboard(MainWindow),
        },
        {
          label: t('export_all_transaction_data'),
          accelerator: 'CommandOrControl+E',
          click: () => MainWindow && exportAllTradingData(MainWindow),
        },
      ],
    },
    {
      label: t('help'),
      submenu: [
        {
          label: t('website'),
          click: () => openUrl('https://www.kungfu-trader.com/'),
        },
        {
          label: t('user_manual'),
          click: () => openUrl('https://www.kungfu-trader.com/manual/'),
        },
        {
          label: t('API_documentation'),
          click: () => openUrl('https://www.kungfu-trader.com/api-doc/'),
        },
        {
          label: t('Kungfu_forum'),
          click: () => openUrl('https://www.kungfu-trader.com/community/'),
        },
      ],
    },
  ];

  Menu.setApplicationMenu(Menu.buildFromTemplate(template));
}

process
  .on('uncaughtException', (err: Error) => {
    kfLogger.error(
      '[MASTER] Error caught in uncaughtException event:',
      err.message,
    );
  })
  .on('unhandledRejection', (err: Error) => {
    kfLogger.error(
      '[MASTER] Error caught in unhandledRejection event:',
      err.message,
    );
  });

registerScheduleTasks(createWindow);
ipcMain.on('schedule-setting-refresh', () => {
  registerScheduleTasks(createWindow);
});
