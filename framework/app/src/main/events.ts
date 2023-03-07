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
    }, 500);

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

export function clearDB(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'clear-db');
}

export function resetMainDashboard(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'reset-main-dashboard');
}

export function openLogFile(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'open-log');
}

export function exportAllTradingData(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'export-all-trading-data');
}

export function foundNewVersion(
  mainWindow: BrowserWindow,
  newVersion: string,
): void {
  sendMsgToMainWindow(mainWindow, 'auto-update-find-new-version', {
    newVersion,
  });
}

export function startDownloadNewVersion(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'auto-update-start-download');
}

export function downloadProcessUpdate(
  mainWindow: BrowserWindow,
  process: number,
): void {
  sendMsgToMainWindow(mainWindow, 'auto-update-download-process', { process });
}

export function updateNotAvailable(mainWindow: BrowserWindow): void {
  sendMsgToMainWindow(mainWindow, 'auto-update-up-to-date');
}

export function sendUpdatingError(
  mainWindow: BrowserWindow,
  error: Error,
): void {
  sendMsgToMainWindow(mainWindow, 'auto-update-error', { error });
}

function sendMsgToMainWindow(
  mainWindow: BrowserWindow,
  msg: string,
  payload?: object,
): void {
  if (mainWindow && !mainWindow.isDestroyed() && mainWindow.webContents) {
    mainWindow.webContents.send('main-process-messages', msg, payload);
    mainWindow.focus();
  }
}
