
// Modules to control application life and create native browser window
import electron from 'electron';

//base setting, init db
import { initConfig, copyKungfuKey } from '__assets/base';
import { killExtra } from '__gUtils/processUtils';
import { logger } from '__gUtils/logUtils';
import { platform } from '__gConfig/platformConfig';
import { openUrl, showKungfuInfo, showQuitMessageBox } from './utils';
import { KF_HOME, KUNGFU_ENGINE_PATH, BASE_DB_DIR, LOG_DIR } from '__gConfig/pathConfig';
import { openSettingDialog, clearMdJournal } from "./events";

const path = require('path');
const { app, globalShortcut, BrowserWindow, Menu, shell } = electron

initConfig();
copyKungfuKey();
setMenu();

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;
var allowQuit = false;
function createWindow () {
	// Create the browser window.
	const electronScreen = electron.screen;    
	const { width, height } = electronScreen.getPrimaryDisplay().size;

	mainWindow = new BrowserWindow({
		show: false,
		width: width > 1920 ? 1920 : width,
		height: height > 1200 ? 1200 : height,
		useContentSize: true,
		webPreferences: {
			nodeIntegration: true,
			nodeIntegrationInWorker: true
		},
		backgroundColor: '#161B2E',
	})

	const isDevelopment = process.env.NODE_ENV === "development" 
	// and load the index.html of the app.
	if(isDevelopment){
		mainWindow.loadURL('http://localhost:9090/index.html')
	}else{
		const filePath = path.join(__dirname, "index.html");
		mainWindow.loadFile(filePath)
	}

	// // Emitted when the window is closed.
	mainWindow.on('close', (e) => {

	// Dereference the window object, usually you would store windows
	// in an array if your app supports multi windows, this is the time
	// when you should delete the corresponding element.
		if (!allowQuit) {
			e.preventDefault();
			showQuitMessageBox(mainWindow)
				.then(res => {
					if (res) {
						allowQuit = true;
					}
				})	
				.catch(err => {
					console.error(err)
				})
		} else {
			return
		}
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
}


//防止重开逻辑
const gotTheLock = app.requestSingleInstanceLock()
if(!gotTheLock) {
	allowQuit = true;
	app.quit()
} else {
	app.on('second-instance', () => {
		if(mainWindow) {
			if(mainWindow.isMinimized()) mainWindow.restore()
			mainWindow.focus()
		}
	})
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
var appReady = false, killExtraFinished = false;
app.on('ready', () => {
	appReady = true;
	if(appReady && killExtraFinished) createWindow();

	globalShortcut.register('CommandOrControl+Shift+I', () => {
		BrowserWindow.getFocusedWindow().webContents.openDevTools()
	})
})

//一上来先把所有之前意外没关掉的 pm2/kfc 进程kill掉
console.time('init clean')
killExtra()
	.finally(() => {
		console.timeEnd('init clean')
		killExtraFinished = true;
		if(appReady && killExtraFinished) createWindow(true)
	})


// Quit when all windows are closed.
app.on('window-all-closed', function (e) {
// On macOS it is common for applications and their menu bar
// to stay active until the user quits explicitly with Cmd + Q
	app.quit()
})

app.on('activate', function () {
// On macOS it's common to re-create a window in the app when the
// dock icon is clicked and there are no other windows open.
    if (mainWindow && mainWindow.isDestroyed()) createWindow()
    else if(mainWindow && mainWindow.isVisible()) mainWindow.focus()
    else mainWindow && mainWindow.show()
})

app.on('will-quit', (e) => {
	if (allowQuit) {
		globalShortcut.unregisterAll()
		return
	};	

	e.preventDefault()
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.

function setMenu() {
    //添加快捷键
	let applicationOptions = [
		{ label: "关于功夫交易", click: () => showKungfuInfo()},
		{ label: "设置", accelerator: "CmdOrCtrl+,", click: () => openSettingDialog(mainWindow)},
		{ label: "关闭", accelerator: "CmdOrCtrl+W", click: () => BrowserWindow.getFocusedWindow().close()}
	]

	if(platform === 'mac') {
		applicationOptions.push(
			{ label: "退出", accelerator: "Command+Q", click: () => app.quit()},
		)
	}

	const template = [
	{
		label: "功夫",
		submenu: applicationOptions
	},{
		//此处必要，不然electron内使用复制粘贴会无效
		label: "编辑",
		submenu: [
			{ label: "复制", accelerator: "CmdOrCtrl+C", selector: "copy:" },
			{ label: "黏贴", accelerator: "CmdOrCtrl+V", selector: "paste:" },
			{ label: "全选", accelerator: "CmdOrCtrl+A", role: "selectall" },
			{ label: "撤销", accelerator: "CmdOrCtrl+Z", role: "undo" }
		]
	},{
		label: "文件",
		submenu: [
			{ label: "打开功夫资源目录（KF_HOME）", accelerator: "CmdOrCtrl+Shift+H",  click: () => shell.showItemInFolder(KF_HOME) },
			{ label: "打开功夫安装目录", accelerator: "CmdOrCtrl+Shift+A", click: () => shell.showItemInFolder(app.getAppPath()) },			
			{ label: "打开功夫基础配置DB", accelerator: "CmdOrCtrl+Shift+B", click: () => shell.showItemInFolder(path.join(BASE_DB_DIR, 'config.db')) },			
		]
	},{
		label: '运行',
		submenu: [
			{ label: "清理journal", accelerator: "CmdOrCtrl+Shift+U", click: () => clearMdJournal(mainWindow)}
		]
	},{
		label: "帮助",
		submenu: [
			{ label: "官网", accelerator: "", click: () => openUrl("https://www.kungfu-trader.com/") },
			{ label: "用户手册", accelerator: "", click: () => openUrl("https://www.kungfu-trader.com/manual/")},
			{ label: "策略API文档", accelerator: "", click: () => openUrl("https://www.kungfu-trader.com/api-doc/") },
			{ label: "Kungfu 论坛", accelerator: "", click: () => openUrl("https://www.kungfu-trader.com/community/") }
		]
	}];
	
	Menu.setApplicationMenu(Menu.buildFromTemplate(template))
}




process.on('uncaughtException', err => {
	console.log(err)
    logger.error('[MASTER] Error caught in uncaughtException event:', err);
});

process.stderr.on('data', err => {
	console.log(err)
    logger.error('[MASTER] Error caught in stderr event:', err);
})
