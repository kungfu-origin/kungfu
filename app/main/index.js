// Modules to control application life and create native browser window
const path = require('path');
const {app, BrowserWindow, Menu, dialog, Tray} = require('electron');
const electron = require('electron');
//base setting, init db
const {initDB, killAll} = require('./base');
const {killAllProcess, killGodDaemon} = require('__gUtils/processUtils');
const {logger} = require('__gUtils/logUtils');

if (process.env.NODE_ENV !== 'development') {
	global.__resources = require('path').join(__dirname, '/resources').replace(/\\/g, '\\\\')// eslint-disable-line{{/if_eq}}
}

//一上来先把所有之前意外没关掉的 pm2/kfc 进程kill掉
killAll().catch(err => console.error(err))



// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function (e) {
// On macOS it is common for applications and their menu bar
// to stay active until the user quits explicitly with Cmd + Q
	if (process.platform !== 'darwin') {
		if(process.platform === 'win32'){
			mainWindow.hide()
		}
	}
})

app.on('activate', function () {
// On macOS it's common to re-create a window in the app when the
// dock icon is clicked and there are no other windows open.
	if (mainWindow === null) createWindow()
})

var allowQuit = false;
app.on('will-quit', (e) => {
	if(!allowQuit) e.preventDefault()
	else return;
	dialog.showMessageBox({
		type: 'question',
		title: '提示',
		message: "退出应用会结束所有交易进程，确认退出吗？",
		buttons: ['是', '否'],
		icon: path.join(__resources, 'icon', 'icon.png')
	}, (index) => {
		if(index === 0){
			//退出所有进程
			// killAllProcess().finally(() => {
				killGodDaemon().finally(() => {
					killAll()
					.catch(err => console.error(err))
					.finally(() => {
						allowQuit = true;
						app.quit();
						appIcon && appIcon.destory && appIcon.destory();
					})
				})
			// })
		}
	})
})

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow, appIcon;
function createWindow () {
	//添加快捷键
	if (process.platform === 'darwin') {
		const template = [
		{
			label: "Application",
			submenu: [
				{ label: "Quit", accelerator: "Command+Q", click: function() { app.quit(); }}
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
	} else {
		Menu.setApplicationMenu(null)
	}
	
	// Create the browser window.
	const electronScreen = electron.screen;    
	const {width,height} = electronScreen.getPrimaryDisplay().size
	mainWindow = new BrowserWindow({
		show: true,
		width,
		height,
		useContentSize: true,
		webPreferences: {
			nodeIntegration: true
		},
		backgroundColor: '#161B2E',
		// titleBarStyle: 'hidden'
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

	// Emitted when the window is closed.
	mainWindow.on('closed', function () {
	// Dereference the window object, usually you would store windows
	// in an array if your app supports multi windows, this is the time
	// when you should delete the corresponding element.
		mainWindow = null
	})


	mainWindow.on('crashed', () => {
		logger.error('crashed', new Date())
		mainWindow && mainWindow.reload()
	})

	mainWindow.on('unresponsive', () => {
		logger.error('unresponsive', new Date())
		mainWindow && mainWindow.reload()
		// dialog.showMessageBox({
		// 	type: 'info',
		// 	title: '主窗口进程无响应',
		// 	message: "主窗口进程无响应！",
		// 	buttons: ['重启', '关闭'],
		// 	icon: path.join(__resources, 'icon', 'icon.png')
		// }, (index) => {
		// 	if(index === 0) mainWindow.reload()
		// 	else mainWindow.close()
		// })
	})



	//create db
	initDB()

	//tray
	console.log('current process:', process.platform)
	const iconName = process.platform === 'win32' ? '20_white@3x.png' : '20.png';
	const iconPath = path.join(__resources, 'icon', iconName)
	const contextMenu = Menu.buildFromTemplate([
		{label: '退出', click: () => app.quit()},//我们需要在这里有一个真正的退出（这里直接强制退出）
	])
	appIcon = appIcon ? appIcon : new Tray(iconPath);
	appIcon.setToolTip('kungfu.trader')
	appIcon.setContextMenu(contextMenu)
	appIcon.on('click', () => {
		if(mainWindow) mainWindow.reload()
		else createWindow()
	})
}

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.