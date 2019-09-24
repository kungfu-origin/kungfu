// Modules to control application life and create native browser window
const path = require('path');
const { app, BrowserWindow, Menu, dialog, ipcMain } = require('electron');
const electron = require('electron');
//base setting, init db
const { initDB } = require('./base');
const { killGodDaemon,  killExtra, killKfc, killKungfu } = require('__gUtils/processUtils');
const { logger } = require('__gUtils/logUtils');
const { platform } = require('__gConfig/platformConfig');
const packageJSON = require('__root/package.json');
const { KF_HOME, KUNGFU_ENGINE_PATH } = require('__gConfig/pathConfig');


// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;
var allowQuit = false;
function createWindow () {
	//添加快捷键
	const template = [
	{
		label: "Kungfu",
		submenu: [
			{ label: "About Kungfu", click: showKungfuInfo},
			{ label: "Quit", accelerator: "Command+Q", click: function() { app.quit(); }},
			{ label: "Close", accelerator: "CmdOrCtrl+W", click: function() { console.log(BrowserWindow.getFocusedWindow().close()); }}
		]
	}, 
	{
		label: "Edit",
		submenu: [
			{ label: "Copy", accelerator: "CmdOrCtrl+C", selector: "copy:" },
			{ label: "Paste", accelerator: "CmdOrCtrl+V", selector: "paste:" },
		]
	}];
	Menu.setApplicationMenu(Menu.buildFromTemplate(template))

	// Create the browser window.
	const electronScreen = electron.screen;    
	const { width,height } = electronScreen.getPrimaryDisplay().size
	mainWindow = new BrowserWindow({
		show: false,
		width,
		height,
		useContentSize: true,
		webPreferences: {
			nodeIntegration: true
		},
		backgroundColor: '#161B2E',
	})
	const isDevelopment = process.env.NODE_ENV === "development" 
	// and load the index.html of the app.
	if(isDevelopment){
		mainWindow.loadURL('http://localhost:9090')
	}else{
		const filePath = path.join(__dirname, "index.html");
		mainWindow.loadFile(filePath)
	}

	// Open the DevTools.
	// mainWindow.webContents.openDevTools()

	// // Emitted when the window is closed.
	mainWindow.on('close', (e) => {
	// Dereference the window object, usually you would store windows
	// in an array if your app supports multi windows, this is the time
	// when you should delete the corresponding element.
		if (platform !== 'mac' && !allowQuit) {
			showQuitMessageBox();	
			e.preventDefault();
		}
		else return
	})

	mainWindow.on('crashed', () => {
		logger.error('crashed', new Date())
		mainWindow && mainWindow.reload()
	})

	mainWindow.on('unresponsive', () => {
		logger.error('unresponsive', new Date())
		mainWindow && mainWindow.reload()
	})

	mainWindow.on('ready-to-show', function() {
		mainWindow.show();
		mainWindow.focus();
	});

	//create db
	initDB()
	// if(platform === 'win') updateHandler(mainWindow)
}


//防止重开逻辑
const gotTheLock = app.requestSingleInstanceLock()
if(!gotTheLock) {
	allowQuit = true;
	app.quit()
} else {
	app.on('second-instance', (event, commandLine, workingDirectory) => {
		if(mainWindow) {
			if(mainWindow.isMinimized()) mainWindow.restore()
			mainWindow.focus()
		}
	})
}



var appReady = false, killExtraFinished = false;
// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', () => {
	appReady = true;
	if(appReady && killExtraFinished) createWindow()
})

//一上来先把所有之前意外没关掉的 pm2/kfc 进程kill掉
console.time('finish kill extra')
killExtra()
.catch(err => console.error(err))
.finally(() => {
	killExtraFinished = true;
	if(appReady && killExtraFinished) createWindow()
	console.timeEnd('finish kill extra')
})

// Quit when all windows are closed.
app.on('window-all-closed', function (e) {
// On macOS it is common for applications and their menu bar
// to stay active until the user quits explicitly with Cmd + Q
	 if (platform !== 'mac') app.quit()
})

app.on('activate', function () {
// On macOS it's common to re-create a window in the app when the
// dock icon is clicked and there are no other windows open.
    if (mainWindow.isDestroyed()) createWindow()
    else if(mainWindow.isVisible()) mainWindow.focus()
    else mainWindow.show()
})

app.on('will-quit', async (e) => {
	if(allowQuit) return
	if(process.env.APP_TYPE === 'test') {
		try {
			await KillAll()
		} catch (err) {
			console.error(err)
		}
		app.quit()
		return;
	}
	if (platform === 'mac') showQuitMessageBox();
	e.preventDefault()
})




// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
function showKungfuInfo() {
	const version = packageJSON.version;
	const electronVersion = packageJSON.devDependencies.electron;
	const info = `Version: ${version}\n`
	+ `electron: ${electronVersion} \n`
	+ `platform: ${platform} \n`
	+ `kfHome: ${KF_HOME} \n`
	+ `kungfuEngine: ${path.resolve(KUNGFU_ENGINE_PATH, 'kfc')} \n`
	+ `commit: ${git_commit_version}`
	dialog.showMessageBox({
		type: 'info',
		message: 'Kungfu',
		defaultId: 0,
		detail: info,
		buttons: ['好的'],
		icon: path.join(__resources, 'icon', 'icon.png')
	})
}

//退出提示
function showQuitMessageBox(){
	dialog.showMessageBox({
		type: 'question',
		title: '提示',
		defaultId: 0,
		cancelId: 1,
		message: "退出应用会结束所有交易进程，确认退出吗？",
		buttons: ['确认', `最小化至${platform !== 'mac' ? '任务栏' : ' Dock'}`],
		icon: path.join(__resources, 'icon', 'icon.png')
	}, (index) => {
		if(index === 0){
			KillAll().then(() => app.quit())
		}else{
			if((mainWindow !== null) && !mainWindow.isDestroyed()){
				if(platform === 'mac') mainWindow.hide();
				else mainWindow.minimize();
			}
		}
	})
}

//结束所有进程
function KillAll(){
	return new Promise(resolve => {
		if(mainWindow && !mainWindow.isDestroyed()) mainWindow.hide()
		allowQuit = true;
		console.time('kill kfcs');
		killKfc()
		.catch(err => console.error(err)) 
		.finally(() => {
			console.timeEnd('kill kfcs');
			if(platform === 'linux') killKungfu()
			console.time('kill daemon');
			killGodDaemon()
			.catch(err => console.error(err)) 				
			.finally(() => {
				console.timeEnd('kill daemon');
				console.time('kill extra');
				killExtra()
				.catch(err => console.error(err)) 								
				.finally(() => {
					console.timeEnd('kill extra');	
					resolve(true)
				})
			})
		})
	})
}




// 注意这个autoUpdater不是electron中的autoUpdater
const { autoUpdater } = require("electron-updater");
const { uploadUrl } = require('__gConfig/updateConfig');

// 检测更新，在你想要检查更新的时候执行，renderer事件触发后的操作自行编写
function updateHandler(mainWindow){
    let message = {
        error: '检查更新出错！',
        checking: '正在检查更新...',
        updateAva: '检测到新版本，正在下载...',
		updateNotAva: '当前为最新版本！',
		downloaded: '下载已完成！'
	};

	autoUpdater.autoInstallOnAppQuit = false;
	autoUpdater.setFeedURL(uploadUrl);
    autoUpdater.on('error', (error) => {
		sendUpdateMessage(mainWindow, message.error)
		return new Error(error)		
	});
    autoUpdater.on('checking-for-update', () => {
        sendUpdateMessage(mainWindow, message.checking)
    });
    autoUpdater.on('update-available', (info) => {
        sendUpdateMessage(mainWindow, message.updateAva)
    });
    autoUpdater.on('update-not-available', (info) => {
        sendUpdateMessage(mainWindow, message.updateNotAva)
    });

    // 更新下载进度事件
    autoUpdater.on('download-progress', (progressObj) => {
        mainWindow.webContents.send('downloadProgress', progressObj)
    })
    autoUpdater.on('update-downloaded', (event, releaseNotes, releaseName, releaseDate, updateUrl, quitAndUpdate) => {
        dialog.showMessageBox({
            type: 'question',
            title: '提示',
            defaultId: 0,
            cancelId: 1,
            message: "检测到新版本，更新会退出应用，是否立即更新？",
            buttons: ['立即更新', '否'],
            icon: path.join(__resources, 'icon', 'icon.png')
        }, (index) => {
            if(index === 0){
				KillAll().then(() => {
					setImmediate(() => {
						autoUpdater.quitAndInstall(); 
						app.quit()
					})
				})
            }
        })
	});
	
	ipcMain.on('checkForUpdate', () => autoUpdater.checkForUpdates().catch(err => console.error(err)))
}

// 通过main进程发送事件给renderer进程，提示更新信息
function sendUpdateMessage(mainWindow, text) {
    mainWindow.webContents.send('message', text)
}