import electron from 'electron';
import { KF_HOME, KUNGFU_ENGINE_PATH } from '__gConfig/pathConfig';
import { killGodDaemon, killKfc, killKungfu } from '__gUtils/processUtils';
import { platform } from '__gConfig/platformConfig';
import { killExtra } from '__gUtils/processUtils';
import { reqRecordBeforeQuit } from "./events";

const path = require('path');
const { app, dialog } = electron
const packageJSON = require('__root/package.json');

export function openUrl(url) {
	electron.shell.openExternal(url)
}




export function showKungfuInfo () {
	const version = packageJSON.version;
	const electronVersion = packageJSON.devDependencies.electron;
	const info = `Version: ${version}\n`
	+ `electron: ${electronVersion} \n`
	+ `python: ${python_version}\n`
	+ `platform: ${platform} \n`
	+ `kungfu_home: ${KF_HOME} \n`
	+ `kungfu_engine: ${path.resolve(KUNGFU_ENGINE_PATH, 'kfc')} \n`
	+ `kungfu_resources: ${path.resolve(KUNGFU_ENGINE_PATH)} \n`
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


//结束所有进程
function KillAll () {
	return new Promise(resolve => {
		killKfc()
		.catch(err => console.error(err)) 
		.finally(() => {
			if(platform === 'linux') killKungfu()
			killGodDaemon()
			.catch(err => console.error(err)) 				
			.finally(() => {
				killExtra()
				.catch(err => console.error(err)) 								
				.finally(() => {
					resolve(true)
				})
			})
		})
	})
}


export function killAllBeforeQuit (mainWindow) {
	if(mainWindow && !mainWindow.isDestroyed()) mainWindow.hide()
	return KillAll()
}

//退出提示
export function showQuitMessageBox (mainWindow) {
    return new Promise(resolve => {
        dialog.showMessageBox({
            type: 'question',
            title: '提示',
            defaultId: 0,
            cancelId: 1,
            message: "退出应用会结束所有交易进程，确认退出吗？",
            buttons: ['确认', '取消'],
            icon: path.join(__resources, 'icon', 'icon.png')
        }, (index) => {
            if(index === 0){
				reqRecordBeforeQuit(mainWindow)
					.then(() => {
						resolve(true)
					})
					.then(() => {
						console.time('quit clean')
						return killAllBeforeQuit(mainWindow).finally(() => {
							console.timeEnd('quit clean')
							app.quit()
						})
					})
              
            }else{
                resolve(false)
            }
        })
    })

}
