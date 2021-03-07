const { ipcMain } = require('electron')

export const reqRecordBeforeQuit = (mainWindow) => {
    return new Promise(resolve => {
	    if (!mainWindow || !mainWindow.webContents) {
            resolve(false)
            return;
        }

        //30s后强制关闭
        console.time('record before quit')
        const timer = setTimeout(() => {
            resolve(false)
            console.timeEnd('record before quit')
            clearTimeout(timer)
        }, 10000)

        mainWindow.webContents.send('main-process-messages', 'record-before-quit')
        
        ipcMain.on('record-before-quit-done', () => {
            console.timeEnd('record before quit')
            resolve(true)
        })
    })
} 

//开启发送renderprocess 打开设置弹窗
export function openSettingDialog (mainWindow) {
	if (mainWindow && mainWindow.webContents) {
		mainWindow.webContents.send('main-process-messages', 'open-setting-dialog')
		mainWindow.focus()
	}
}