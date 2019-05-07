// Modules to control application life and create native browser window
const path = require('path');
const {app, BrowserWindow, Menu, dialog, Tray} = require('electron');
const electron = require('electron');
//base setting, init db
const {initDB, killAll} = require('./base');
const {killGodDaemon} = require('__gUtils/processUtils');
const {logger} = require('__gUtils/logUtils');
const {platform} = require('__gConfig/platformConfig');

if (process.env.NODE_ENV !== 'development') {
	global.__resources = require('path').join(__dirname, '/resources').replace(/\\/g, '\\\\')// eslint-disable-line{{/if_eq}}
}

//一上来先把所有之前意外没关掉的 pm2/kfc 进程kill掉
killAll().catch(err => console.error(err))

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null, appIcon = null;
function createWindow () {
	//添加快捷键
	if (platform === 'mac') {
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
		mainWindow = null;
	})

	mainWindow.on('crashed', () => {
		logger.error('crashed', new Date())
		mainWindow && mainWindow.reload()
	})

	mainWindow.on('unresponsive', () => {
		logger.error('unresponsive', new Date())
		mainWindow && mainWindow.reload()
	})

	//create db
	initDB()

	//tray
	console.log('current process:', process.platform)
	const iconName = platform === 'win' ? '20_white@3x.png' : '20.png';
	const iconPath = path.join(__resources, 'icon', iconName)
	const contextMenu = Menu.buildFromTemplate([
		{label: '重新加载  ', click: () => mainWindow ? mainWindow.show() : createWindow()},
		{label: '退出  ', click: () => app.quit()},//我们需要在这里有一个真正的退出（这里直接强制退出）
	])
	appIcon = appIcon ? appIcon : new Tray(iconPath);
	appIcon.setToolTip('kungfu.trader')
	appIcon.setContextMenu(contextMenu)
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function (e) {
// On macOS it is common for applications and their menu bar
// to stay active until the user quits explicitly with Cmd + Q
	if (platform !== 'mac') {
		if(platform === 'win'){
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
	if(allowQuit) return
	else e.preventDefault()
	dialog.showMessageBox({
		type: 'question',
		title: '提示',
		message: "退出应用会结束所有交易进程，确认退出吗？",
		buttons: ['是', '否'],
		icon: path.join(__resources, 'icon', 'icon.png')
	}, async(index) => {
		if(index === 0){
			appIcon && appIcon.destory && appIcon.destory();
			appIcon = null;
			console.log('starting quit process ')
			console.time('kill daemon')
			try {
				await killGodDaemon();
			} catch (err) {
				console.error(err)
			}
			console.timeEnd('kill daemon')

			console.time('kill all')
			try {
				await killAll()
			} catch (err) {
				console.error(err)
			}
			console.timeEnd('kill all')

			allowQuit = true;
			app.quit();
		}
	})
})


// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.

