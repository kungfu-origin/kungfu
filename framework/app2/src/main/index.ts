import { BrowserWindow, screen, app, globalShortcut } from 'electron';
import path from 'path';
import { showQuitMessageBox, showCrashMessageBox } from '@main/utils';
import { logger } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { killKfc } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import * as remoteMain from '@electron/remote/main';

remoteMain.initialize();

let MainWindow: BrowserWindow | null = null;
let AllowQuit = false;
let CrashedReloading = false;
const isDev = process.env.NODE_ENV === 'development';

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
            webSecurity: false,
            nodeIntegration: true,
            nodeIntegrationInWorker: true,
            contextIsolation: false,
            enableRemoteModule: true,
            additionalArguments: [
                reloadAfterCrashed ? 'reloadAfterCrashed' : '',
            ],
        },
        backgroundColor: '#161B2E',
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
        if (CrashedReloading) {
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
            return;
        }
    });

    MainWindow.on('unresponsive', () => {
        logger.error('[MainWindow] unresponsive', new Date());
        if (AllowQuit) return;
        showCrashMessageBox().then((confirm) => {
            if (!confirm) return;
            createWindow(true);
        });
    });

    MainWindow.webContents.on('render-process-gone', (event, details) => {
        logger.error('[MainWindow.webContents] crashed', new Date(), details);
        if (AllowQuit) return;
        showCrashMessageBox().then((confirm) => {
            if (!confirm) return;
            createWindow(true);
        });
    });

    MainWindow.webContents.on('unresponsive', () => {
        logger.error('[MainWindow.webContents] unresponsive', new Date());
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
killKfc().finally(() => {
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
