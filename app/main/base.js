const path = require('path')
const sqlite3 = require('sqlite3').verbose();
const fse = require('fs-extra');
const initGobalDB = require('__gConfig/initGlobalDB.json');
const {GLOBAL_DIR} = require('__gConfig/pathConfig');
const {logger} = require('__gUtils/logUtils');

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!fse.existsSync(GLOBAL_DIR)){
        fse.mkdirSync(GLOBAL_DIR)
    }

    //循环建立表
    Object.keys(initGobalDB).forEach((dbName) => {
        const db = new sqlite3.Database(path.join(GLOBAL_DIR, `${dbName}.db`));
        const tables = initGobalDB[dbName];
        db.serialize(() => {
            tables.forEach((table) => {
                db.run(table.sql)
            })
        })	
        db.close();
    })

    //commission.db
    fse.copy(path.join(__resources, 'default', 'commission.db'), path.join(GLOBAL_DIR, 'commission.db'), err => {
        if(err) logger.error(err);
    })

    //holidays.db
    fse.copy(path.join(__resources, 'default', 'holidays.db'), path.join(GLOBAL_DIR, 'holidays.db'), err => {
        if(err) logger.error(err);
    })
}


// 注意这个autoUpdater不是electron中的autoUpdater
const { autoUpdater } = require("electron-updater");
const { uploadUrl } = require('__gConfig/updateConfig');

// 检测更新，在你想要检查更新的时候执行，renderer事件触发后的操作自行编写
export const updateHandle = () => {
    let message = {
        error: '检查更新出错',
        checking: '正在检查更新...',
        updateAva: '检测到新版本，正在下载...',
        updateNotAva: '现在使用的就是最新版本，不用更新',
    };

    autoUpdater.setFeedURL(uploadUrl);
    autoUpdater.on('error', (error) => {
        sendUpdateMessage(message.error)
    });
    autoUpdater.on('checking-for-update', () => {
        sendUpdateMessage(message.checking)
    });
    autoUpdater.on('update-available', (info) => {
        sendUpdateMessage(message.updateAva)
    });
    autoUpdater.on('update-not-available', (info) => {
        sendUpdateMessage(message.updateNotAva)
    });

    // 更新下载进度事件
    autoUpdater.on('download-progress', (progressObj) => {
        mainWindow.webContents.send('downloadProgress', progressObj)
    })
    autoUpdater.on('update-downloaded', (event, releaseNotes, releaseName, releaseDate, updateUrl, quitAndUpdate) => {
        ipcMain.on('isUpdateNow', (e, arg) => {
            console.log(arguments);
            console.log("开始更新");
            //some code here to handle event
            autoUpdater.quitAndInstall();
        });
        mainWindow.webContents.send('isUpdateNow')
    });

    ipcMain.on("checkForUpdate",() => {
        //执行自动更新检查
        autoUpdater.checkForUpdates();
    })
}

// 通过main进程发送事件给renderer进程，提示更新信息
function sendUpdateMessage(text) {
    mainWindow.webContents.send('message', text)
}