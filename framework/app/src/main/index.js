
// Modules to control application life and create native browser window
import electron from 'electron';

//base setting, init db
import { initConfig, copyKungfuKey } from '@kungfu-trader/kungfu-uicc/assets/base';
import { killExtra } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';
import { platform } from '@kungfu-trader/kungfu-uicc/config/platformConfig';
import { openUrl, showKungfuInfo, showQuitMessageBox, showCrashMessageBox } from './utils';
import { KF_HOME, BASE_DB_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { openSettingDialog, clearJournal, openLogFile, exportAllTradingData } from "./events";

const path = require('path');
const { app, globalShortcut, BrowserWindow, Menu, shell } = electron

initConfig();
copyKungfuKey();
setMenu();
require('@electron/remote/main').initialize()


// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var MainWindow = null;
var AllowQuit = false;
var CrashedReloading = false;

function createWindow (reloadAfterCrashed = false) {

	if (reloadAfterCrashed) {
		MainWindow && MainWindow.destroy();
		CrashedReloading = true;
	}

	// Create the browser window.
	const electronScreen = electron.screen;    
	const { width, height } = electronScreen.getPrimaryDisplay().size;

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
				reloadAfterCrashed ? "reloadAfterCrashed": ""
			]
		},
		backgroundColor: '#161B2E',
	})



	const isDevelopment = process.env.NODE_ENV === "development" 
	// and load the index.html of the app.
	if(isDevelopment){
		MainWindow.loadURL('http://localhost:9090/index.html')
	}else{
		const filePath = path.join(__dirname, "index.html");
		MainWindow.loadFile(filePath)
	}

	MainWindow.on('ready-to-show', function() {
		MainWindow.show();
		MainWindow.focus();
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
			showQuitMessageBox(MainWindow)
				.then(res => {
					if (res) {
						AllowQuit = true;
					}
				})	
				.catch(err => {
					console.error(err)
				})
		} else {
			return
		}
	})

	MainWindow.on('crashed', () => {
		logger.error('[MainWindow] crashed', new Date())
		showCrashMessageBox().then((confirm) => {
			if (!confirm) return;
			createWindow(true);
		})
	});

	MainWindow.on('unresponsive', () => {
		logger.error('[MainWindow] unresponsive', new Date())
		showCrashMessageBox().then((confirm) => {
			if (!confirm) return;
			createWindow(true);
		})
	});

	MainWindow.webContents.on("render-process-gone", (event, details) => {
		logger.error("[MainWindow.webContents] crashed", new Date(), details);
		showCrashMessageBox().then((confirm) => {
			if (!confirm) return;
			createWindow(true);
		})
	})

	MainWindow.webContents.on("unresponsive", () => {
		logger.error("[MainWindow.webContents] unresponsive", new Date());
		showCrashMessageBox().then((confirm) => {
			if (!confirm) return;
			createWindow(true);
		})
	})
}


//防止重开逻辑
const gotTheLock = app.requestSingleInstanceLock()
if(!gotTheLock) {
	AllowQuit = true;
	app.quit()
} else {
	app.on('second-instance', () => {
		if(MainWindow) {
			if(MainWindow.isMinimized()) MainWindow.restore()
			MainWindow.focus()
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
		if(appReady && killExtraFinished) createWindow()
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
    if (MainWindow && MainWindow.isDestroyed()) createWindow()
    else if(MainWindow && MainWindow.isVisible()) MainWindow.focus()
    else MainWindow && MainWindow.show()
})

app.on('will-quit', (e) => {
	if (AllowQuit) {
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
		{ label: "设置", accelerator: "CommandOrControl+,", click: () => openSettingDialog(MainWindow)},
		{ label: "关闭", accelerator: "CommandOrControl+W", click: () => BrowserWindow.getFocusedWindow().close()}
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
			{ label: "复制", accelerator: "CommandOrControl+C", selector: "copy:" },
			{ label: "黏贴", accelerator: "CommandOrControl+V", selector: "paste:" },
			{ label: "全选", accelerator: "CommandOrControl+A", role: "selectall" },
			{ label: "撤销", accelerator: "CommandOrControl+Z", role: "undo" }
		]
	},{
		label: "文件",
		submenu: [
			{ label: "打开功夫资源目录（KF_HOME）", click: () => shell.showItemInFolder(KF_HOME) },
			{ label: "打开功夫安装目录", click: () => shell.showItemInFolder(app.getAppPath()) },			
			{ label: "打开功夫基础配置DB", click: () => shell.showItemInFolder(path.join(BASE_DB_DIR, 'config.db')) },			
			{ label: "浏览日志文件", accelerator: "CommandOrControl+L", click: () => openLogFile(MainWindow) },			
		]
	},{
		label: '运行',
		submenu: [
			{ label: "清理journal", click: () => clearJournal(MainWindow)},
			{ label: "导出所有交易数据", accelerator: "CommandOrControl+E", click: () => exportAllTradingData(MainWindow)}
		]
	},{
		label: "帮助",
		submenu: [
			{ label: "官网", click: () => openUrl("https://www.kungfu-trader.com/") },
			{ label: "用户手册", click: () => openUrl("https://www.kungfu-trader.com/manual/")},
			{ label: "策略API文档", click: () => openUrl("https://www.kungfu-trader.com/api-doc/") },
			{ label: "Kungfu 论坛", click: () => openUrl("https://www.kungfu-trader.com/community/") }
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
