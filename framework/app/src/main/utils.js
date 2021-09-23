import electron from 'electron';
import { KF_HOME, KFC_PARENT_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { killGodDaemon, killKfc, killKungfu } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { platform } from '@kungfu-trader/kungfu-uicc/config/platformConfig';
import { killExtra } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { reqRecordBeforeQuit } from "./events";
import packageJSON from '../../package.json';

const path = require('path');
const { app, dialog } = electron

export function openUrl(url) {
	electron.shell.openExternal(url)
}


export function showKungfuInfo () {
	const version = packageJSON.version;
	const electronVersion = packageJSON.devDependencies.electron;
	const info = 
  `version: ${version}\n` +
  `electron_version: ${electronVersion} \n` +
  `python_version: ${python_version}\n` +
  `platform: ${platform} \n` +
  `kungfu_home: ${KF_HOME} \n` +
  `kungfu_engine: ${path.resolve(KFC_PARENT_DIR, 'kfc')} \n` +
  `kungfu_resources: ${path.resolve(KFC_PARENT_DIR)} \n` +
  `commit_version: ${git_commit_version}\n` +
  `build_timestamp: ${build_timestamp}`;
	dialog.showMessageBox({
		type: 'info',
		message: 'Kungfu',
		defaultId: 0,
		detail: info,
		buttons: ['好的'],
		icon: path.join(__resources, 'logo', 'icon.png')
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
            icon: path.join(__resources, 'logo', 'icon.png')
        })
        .then(({ response }) => {
            if(response === 0){
              reqRecordBeforeQuit(mainWindow)
                .then(() => {
                  resolve(true)
                })
                .then(() => {
                  console.time('quit clean')
                  return killAllBeforeQuit(mainWindow)
                    .finally(() => {
                      console.timeEnd('quit clean')
                      app.quit()
                    })
                })
            } else {
                resolve(false)
            }
        })
    })
}

//崩溃提示
export function showCrashMessageBox () {
	return new Promise(resolve => {
        dialog.showMessageBox({
            type: 'question',
            title: '提示',
            defaultId: 0,
            cancelId: 1,
            message: "功夫图形进程中断，该中断不会影响交易，重新开启后出于安全考虑不会恢复之前交易数据，但可通过历史查询查看，是否重新开启界面？",
            buttons: ['确认', '取消'],
            icon: path.join(__resources, 'logo', 'icon.png')
        })
        .then(({ response }) => {
          if(response === 0){
              resolve(true)
          } else {
              resolve(false)
          }
      })
    })
}
