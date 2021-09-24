const { ipcMain } = require('electron')

export const reqRecordBeforeQuit = (mainWindow) => {
    return new Promise(resolve => {
	    if (!mainWindow || !mainWindow.webContents) {
            resolve(false)
            return;
        }

        //30s后强制关闭
        console.time('record before quit')
        let timer = setTimeout(() => {
            resolve(false)
            console.timeEnd('record before quit');
            console.log("record before quit timeout");
            clearTimeout(timer)
        }, 3000)

        sendMsgToMainWindow(mainWindow, 'record-before-quit')
        
        ipcMain.on('record-before-quit-done', () => {
            resolve(true)
            if (!timer) return; // if timer has been cleared
            console.timeEnd('record before quit')
            clearTimeout(timer)
        })
    })
} 

//开启发送renderprocess 打开设置弹窗
export function openSettingDialog (mainWindow) {
    sendMsgToMainWindow(mainWindow, 'open-setting-dialog')
}

export function clearJournal (mainWindow) {
    sendMsgToMainWindow(mainWindow, 'clear-journal')
}

export function openLogFile (mainWindow) {
    sendMsgToMainWindow(mainWindow, 'open-log')
}

export function exportAllTradingData (mainWindow) {
    sendMsgToMainWindow(mainWindow, 'export-all-trading-data')
}

function sendMsgToMainWindow (mainWindow, msg) {
    if (mainWindow && mainWindow.webContents) {
		mainWindow.webContents.send('main-process-messages', msg)
		mainWindow.focus()
	}
}