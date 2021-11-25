import { BrowserWindow } from 'electron';
import { ipcMain } from 'electron';

export function reqRecordBeforeQuit(
    mainWindow: BrowserWindow,
): Promise<boolean> {
    return new Promise((resolve) => {
        if (!mainWindow || !mainWindow.webContents) {
            resolve(false);
            return;
        }

        //30s后强制关闭
        console.time('record before quit');
        const timer = setTimeout(() => {
            resolve(false);
            console.timeEnd('record before quit');
            console.log('record before quit timeout');
            clearTimeout(timer);
        }, 3000);

        sendMsgToMainWindow(mainWindow, 'record-before-quit');

        ipcMain.on('record-before-quit-done', () => {
            resolve(true);
            if (!timer) return; // if timer has been cleared
            console.timeEnd('record before quit');
            clearTimeout(timer);
        });
    });
}

export function clearProcessBeforeQuitStart(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'clear-process-before-quit-start');
}

export function clearProcessBeforeQuitEnd(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'clear-process-before-quit-end');
}

export function openSettingDialog(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'open-setting-dialog');
}

export function clearJournal(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'clear-journal');
}

export function openLogFile(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'open-log');
}

export function exportAllTradingData(mainWindow: BrowserWindow): void {
    sendMsgToMainWindow(mainWindow, 'export-all-trading-data');
}

function sendMsgToMainWindow(mainWindow: BrowserWindow, msg: string): void {
    if (mainWindow && !mainWindow.isDestroyed() && mainWindow.webContents) {
        mainWindow.webContents.send('main-process-messages', msg);
        mainWindow.focus();
    }
}
