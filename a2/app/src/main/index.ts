import {
    BrowserWindow,
    screen,
    app,
    globalShortcut,
    Menu,
    MenuItemConstructorOptions,
    shell,
} from 'electron';
import * as remoteMain from '@electron/remote/main';
import path from 'path';
import os from 'os';
import {
    // showQuitMessageBox,
    showCrashMessageBox,
    showKungfuInfo,
    openUrl,
} from '@main/utils';
import { kfLogger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { killExtra } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
    clearJournal,
    exportAllTradingData,
    openLogFile,
    openSettingDialog,
} from './events';
import {
    BASE_DB_DIR,
    KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';

let MainWindow: BrowserWindow | null = null;
let AllowQuit = false;
let CrashedReloading = false;
const isDev = process.env.NODE_ENV === 'development';
const isMac = os.platform() === 'darwin';

remoteMain.initialize();
setMenu();

function createWindow(reloadAfterCrashed = false) {
    if (reloadAfterCrashed) {
        MainWindow && MainWindow.destroy();
        CrashedReloading = true;
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
    });

    MainWindow.on('close', (e) => {
        console.log(e);
        AllowQuit = true;
        return;

        // if (CrashedReloading) {
        //     return;
        // }

        // if (!AllowQuit) {
        //     e.preventDefault();
        //     if (MainWindow) {
        //         showQuitMessageBox(MainWindow)
        //             .then((res) => {
        //                 if (res) {
        //                     AllowQuit = true;
        //                 }
        //             })
        //             .catch((err) => {
        //                 console.error(err);
        //             });
        //     }
        // } else {
        //     return;
        // }
    });

    MainWindow.webContents.on('render-process-gone', (event, details) => {
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
const gotTheLock = app.requestSingleInstanceLock();
if (!gotTheLock) {
    AllowQuit = true;
    app.quit();
} else {
    app.on('second-instance', () => {
        if (MainWindow) {
            if (MainWindow.isMinimized()) MainWindow.restore();
            MainWindow.focus();
        }
    });
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
    app.quit();
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
        { label: '关于功夫交易', click: () => showKungfuInfo() },
        {
            label: '设置',
            accelerator: 'CommandOrControl+,',
            click: () => MainWindow && openSettingDialog(MainWindow),
        },
        {
            label: '关闭',
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
            label: '退出',
            accelerator: 'Command+Q',
            click: () => app.quit(),
        });
    }

    const template: MenuItemConstructorOptions[] = [
        {
            label: '功夫',
            submenu: applicationOptions,
        },
        {
            //此处必要，不然electron内使用复制粘贴会无效
            label: '编辑',
            submenu: [
                {
                    label: '复制',
                    accelerator: 'CommandOrControl+C',
                    role: 'copy',
                },
                {
                    label: '黏贴',
                    accelerator: 'CommandOrControl+V',
                    role: 'paste',
                },
                {
                    label: '全选',
                    accelerator: 'CommandOrControl+A',
                    role: 'selectAll',
                },
                {
                    label: '撤销',
                    accelerator: 'CommandOrControl+Z',
                    role: 'undo',
                },
            ],
        },
        {
            label: '文件',
            submenu: [
                {
                    label: '打开功夫资源目录（KF_HOME）',
                    click: () => shell.showItemInFolder(KF_HOME),
                },
                {
                    label: '打开功夫安装目录',
                    click: () => shell.showItemInFolder(app.getAppPath()),
                },
                {
                    label: '打开功夫基础配置DB',
                    click: () =>
                        shell.showItemInFolder(
                            path.join(BASE_DB_DIR, 'config.db'),
                        ),
                },
                {
                    label: '浏览日志文件',
                    accelerator: 'CommandOrControl+L',
                    click: () => MainWindow && openLogFile(MainWindow),
                },
            ],
        },
        {
            label: '运行',
            submenu: [
                {
                    label: '清理journal',
                    click: () => MainWindow && clearJournal(MainWindow),
                },
                {
                    label: '导出所有交易数据',
                    accelerator: 'CommandOrControl+E',
                    click: () => MainWindow && exportAllTradingData(MainWindow),
                },
            ],
        },
        {
            label: '帮助',
            submenu: [
                {
                    label: '官网',
                    click: () => openUrl('https://www.kungfu-trader.com/'),
                },
                {
                    label: '用户手册',
                    click: () =>
                        openUrl('https://www.kungfu-trader.com/manual/'),
                },
                {
                    label: '策略API文档',
                    click: () =>
                        openUrl('https://www.kungfu-trader.com/api-doc/'),
                },
                {
                    label: 'Kungfu 论坛',
                    click: () =>
                        openUrl('https://www.kungfu-trader.com/community/'),
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
