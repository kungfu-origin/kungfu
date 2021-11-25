import os from 'os';
import { app, BrowserWindow, dialog, nativeImage, shell } from 'electron';
import {
    reqRecordBeforeQuit,
    clearProcessBeforeQuitStart,
    clearProcessBeforeQuitEnd,
} from './events';
import path from 'path';
import {
    killKfc,
    killKungfu,
    pm2Kill,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
    delayMilliSeconds,
    kfLogger,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
    KFC_DIR,
    KFC_PARENT_DIR,
    KF_HOME,
} from '@kungfu-trader/kungfu-js-api/config/pathConfig';

const packageJSON = require('@root/package.json');

let BeforeQuitLoading = false;

export function openUrl(url: string): void {
    shell.openExternal(url);
}

export function showKungfuInfo(): void {
    const version = packageJSON.version;
    const electronVersion = packageJSON.devDependencies.electron;
    const info =
        `Version: ${version}\n` +
        `electron: ${electronVersion} \n` +
        `python: ${__python_version}\n` +
        `platform: ${os.platform()} \n` +
        `kungfu_home: ${KF_HOME} \n` +
        `kungfu_engine: ${KFC_DIR} \n` +
        `kungfu_resources: ${KFC_PARENT_DIR} \n` +
        `commit: ${__git_commit_version}` +
        `build_time: ${__build_timestamp}`;
    dialog.showMessageBox({
        type: 'info',
        message: 'Kungfu',
        defaultId: 0,
        detail: info,
        buttons: ['好的'],
        icon: nativeImage.createFromPath(
            path.join(__resources, 'logo', 'icon.png'),
        ),
    });
}

function KillAll(): Promise<void> {
    return new Promise((resolve) => {
        pm2Kill()
            .catch((err) => kfLogger.error(err.message))
            .finally(() => {
                killKfc()
                    .catch((err) => kfLogger.error(err.message))
                    .finally(() => {
                        killKungfu()
                            .catch((err) => kfLogger.error(err.message))
                            .finally(() => {
                                resolve();
                            });
                    });
            });
    });
}

export function killAllBeforeQuit(mainWindow: BrowserWindow): Promise<void> {
    console.time('quit clean');
    clearProcessBeforeQuitStart(mainWindow);
    return KillAll().finally(() => {
        console.timeEnd('quit clean');
        clearProcessBeforeQuitEnd(mainWindow);
    });
}

export function showQuitMessageBox(
    mainWindow: BrowserWindow,
): Promise<boolean> {
    if (BeforeQuitLoading) {
        return Promise.reject(new Error('On Quitting Process'));
    }
    BeforeQuitLoading = true;
    return new Promise((resolve) => {
        dialog
            .showMessageBox({
                type: 'question',
                title: '提示',
                defaultId: 0,
                cancelId: 1,
                message: '退出应用会结束所有交易进程，确认退出吗？',
                buttons: ['确认', '取消'],
                icon: nativeImage.createFromPath(
                    path.join(__resources, 'logo', 'icon.png'),
                ),
            })
            .then(({ response }) => {
                if (response === 0) {
                    return Promise.all([
                        reqRecordBeforeQuit(mainWindow),
                        killAllBeforeQuit(mainWindow),
                    ]).finally(() => {
                        resolve(true);
                        delayMilliSeconds(1000).then(() => {
                            BeforeQuitLoading = false;
                            app.quit();
                        });
                    });
                } else {
                    resolve(false);
                }
            });
    });
}

export function showCrashMessageBox(): Promise<boolean> {
    return dialog
        .showMessageBox({
            type: 'question',
            title: '提示',
            defaultId: 0,
            cancelId: 1,
            message: '功夫图形进程中断，该中断不会影响交易，是否重启图形进程？',
            buttons: ['确认', '取消'],
            icon: nativeImage.createFromPath(
                path.join(__resources, 'logo', 'icon.png'),
            ),
        })
        .then(({ response }) => {
            if (response === 0) {
                return true;
            } else {
                return false;
            }
        });
}
