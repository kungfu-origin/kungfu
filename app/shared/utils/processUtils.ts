/*
 * @Author: your name
 * @Date: 2020-04-24 16:36:28
 * @LastEditTime: 2020-05-28 17:12:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/shared/utils/processUtils.ts
 */ 

//@ts-ignore
import * as taskkill from 'taskkill';
//@ts-ignore
import * as physicalCpuCount from 'physical-cpu-count';

import { KF_HOME, KUNGFU_ENGINE_PATH, KF_CONFIG_PATH, buildProcessLogPath } from '__gConfig/pathConfig';
import { platform } from '__gConfig/platformConfig';
import { logger } from '__gUtils/logUtils';
import { readJsonSync } from '__gUtils/fileUtils';
import { setTimerPromiseTask, delayMiliSeconds } from '__gUtils/busiUtils';
import { getProcesses } from 'getprocesses';


const path = require('path');
const fkill = require('fkill');
const pm2 = require('pm2');

export const _pm2 = pm2;

//=========================== task kill =========================================
export const findProcessByKeywords = (tasks: string[]): Promise<any> => {
    let pIdList: number[] = [];
    return getProcesses().then(processes => {
        processes.forEach(p => {
            const rawCommandLine = p.rawCommandLine
            tasks.forEach((task: string): void => {
                if (rawCommandLine.includes(task)) {
                    pIdList.push(p.pid)
                }
            })
        })
        return pIdList
    })
}

const winKill = async (tasks: string[]): Promise<any> => {
    try {
        const pIdList: any = await findProcessByKeywords(tasks);
        if (!pIdList || !pIdList.length) return new Promise(resolve => resolve(true))
        return taskkill(pIdList, {
            force: true,
            tree: true
        })
    } catch (err) {
        throw err
    }
}

const macKill = (tasks: string[]): any => {
    //@ts-ignore
    return fkill(tasks, {
        force: true,
        silent: true,
        ignoreCase: true
    })
}

const linuxKill = async (tasks: string[]): Promise<any> => {
    try {
        const pIdList: any = await findProcessByKeywords(tasks);
        if (!pIdList || !pIdList.length) return new Promise(resolve => resolve(true))
        //@ts-ignore
        return fkill(pIdList, {
            force: true,
            silent: true,
            ignoreCase: true
        })
    } catch (err) {
        throw err
    }
}

export const kfKill = (tasks: string[]): any => {
    if (platform === 'mac') return macKill(tasks)
    else if (platform === 'linux') return linuxKill(tasks)
    else return winKill(tasks)
}

const kfc = platform === 'win' ? 'kfc.exe' : 'kfc';

export const killKfc = () => kfKill([kfc])

export const killKungfu = () => kfKill(['kungfu'])

export const killExtra = () => kfKill([kfc, 'pm2'])


//=========================== pm2 manager =========================================

const pm2Connect = (): Promise<void> => {
    return new Promise((resolve, reject) => {
        try {
            let noDaemon = platform === 'win' ? true : false
            if (process.env.NODE_ENV !== 'production') noDaemon = false;
            pm2.connect(noDaemon, (err: any): void => {
                if (err) {
                    err = err.length ? err[0] : err;
                    logger.error('[pm2Connect]', err);
                    pm2.disconnect();
                    reject(err);
                    process.exit(2);
                }
                resolve()
            })
        } catch (err) {
            pm2.disconnect()
            logger.error('[TC pm2Connect]', err)
            reject(err)
        }
    })
}

const pm2List = (): Promise<any[]> => {
    return new Promise((resolve, reject) => {
        try {
            pm2.list((err: any, pList: any[]): void => {
                if (err) {
                    err = err.length ? err[0] : err;
                    logger.error('[pm2List]', err)
                    reject(err)
                    return;
                }
                resolve(pList)
            })
        } catch (err) {
            logger.error('[TC pm2List]', err)
            reject(err)
        }
    })
}

const dealSpaceInPath = (pathname: string): string => {
    const normalizePath = path.normalize(pathname);
    return normalizePath.replace(/ /g, '\ ')
}

export const describeProcess = (name: string): Promise<any> => {
    return new Promise((resolve, reject) => {
        //此处无需connect，不然windows会卡死
        try {
            pm2.describe(name, (err: any, res: object): void => {
                if (err) {
                    err = err.length ? err[0] : err;
                    logger.error('[describeProcess]', err)
                    reject(err);
                    return;
                }
                resolve(res)
            })
        } catch (err) {
            logger.error('[TC describeProcess]', err)
            reject(err)
        }
    })

}

function getRocketParams(name: String, ifRocket: Boolean) {
    let rocket = ifRocket ? '-x' : '';
    if ((name === 'master') || (name === 'ledger')) {
        if (physicalCpuCount <= 4) {
            rocket = '';
        }
    }
    return rocket
}

function parseScriptName () {
    if (process.env.NODE_ENV === 'production') {
        const extensionName = platform === 'win' ? '.exe' : ''
        return `index${extensionName}`
    } else {
        return `index.js`
    }
}

export const startTask = (options: any) => {
    const script = parseScriptName();
    const optionsResolved = {
        "logType": "json",
        "output": buildProcessLogPath(options.name),
        "error": buildProcessLogPath(options.name),
        "mergeLogs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": false,
        "maxRestarts": 1,
        "watch": false,
        "force": true,
        "execMode": "fork",
        "killTimeout": 16000,
        ...options,
        script
    }


    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.start(optionsResolved, (err: any, apps: object): void => {
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error('[startProcess]', JSON.stringify(options), err)
                        reject(err);
                        return;
                    };
                    resolve(apps);
                })
            } catch (err) {
                logger.error('[TC startProcess]', JSON.stringify(options), err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

export const startProcess = (options: any, no_ext = false): Promise<object> => {
    const extensionName = platform === 'win' ? '.exe' : ''
    const kfConfig: any = readJsonSync(KF_CONFIG_PATH) || {}
    const ifRocket = ((kfConfig.performance || {}).rocket) || false;
    const logLevel: string = ((kfConfig.log || {}).level) || '';
    const rocket = getRocketParams(options.name, ifRocket);
    const args = [logLevel, options.args, rocket].join(' ')

    options = {
        ...options,
        "args": args,
        "script": `kfc${extensionName}`,
        "cwd": path.join(KUNGFU_ENGINE_PATH, 'kfc'),
        "logType": "json",
        "output": buildProcessLogPath(options.name),
        "error": buildProcessLogPath(options.name),
        "mergeLogs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": false,
        "maxRestarts": 1,
        "watch": false,
        "force": true,
        "execMode": "fork",
        "env": {
            "KF_HOME": dealSpaceInPath(KF_HOME),
        },
        "killTimeout": 16000
    };

    if (no_ext) options['env']['KF_NO_EXT'] = 'on';

    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.start(options, (err: any, apps: object): void => {
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error('[startProcess]', JSON.stringify(options), err)
                        reject(err);
                        return;
                    };
                    resolve(apps);
                })
            } catch (err) {
                logger.error('[TC startProcess]', JSON.stringify(options), err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

export const startStrategyProcess = async (name: string, strategyPath: string, pythonPath: string): Promise<object> => {
    const kfConfig: any = readJsonSync(KF_CONFIG_PATH) || {}
    const ifRocket = ((kfConfig.performance || {}).rocket) || false;
    const logLevel: string = ((kfConfig.log || {}).level) || '';
    const rocket = ifRocket ? '-x' : '';
    const args = ['-m', 'kungfu', logLevel, 'strategy', '-n', name, '-p', `'${strategyPath}'`, rocket].join(' ')

    if (!pythonPath.trim()) {
        return Promise.reject(new Error('No local python path!'))
    }

    const fullPythonPathList = pythonPath.split('/');
    const pythonFolder = fullPythonPathList.slice(0, fullPythonPathList.length - 1).join('/')
    const pythonFile = fullPythonPathList.slice(fullPythonPathList.length - 1).join('/')

    const optionsForPython = {
        "name": name,
        "args": args,
        "cwd": pythonFolder,
        "script": pythonFile,
        "logType": "json",
        "output": buildProcessLogPath(name),
        "error": buildProcessLogPath(name),
        "mergeLogs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": false,
        "maxRestarts": 1,
        "watch": false,
        "force": true,
        "execMode": "fork",
        "env": {
            "KF_HOME": dealSpaceInPath(KF_HOME),
        },
        "killTimeout": 16000
    };

    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.start(optionsForPython, (err: any, apps: object): void => {
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error('[startProcess]', JSON.stringify(optionsForPython), err)
                        reject(err);
                        return;
                    };
                    resolve(apps);
                })
            } catch (err) {
                logger.error('[TC startProcess]', JSON.stringify(optionsForPython), err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


export const startArchiveMake = () => {
    return startProcess({
        "name": 'archive',
        "args": "archive make"
    }, true)
    .then(res => res)
    .catch(err => logger.error('[startArchiveMake]', err))
}


export function startArchiveMakeTask (cb?: Function) {
    return new Promise(resolve => {
        startArchiveMake()
            .then(() => {
                let timer = startGetProcessStatusByName('archive', (res: any[]) => {
                    const archiveStatus = ((res[0] || {}).pm2_env || {}).status;
                    cb && cb(archiveStatus);
                    if (archiveStatus !== 'online') {
                        timer.clearLoop();
                        resolve(archiveStatus)
                    }
                })
            });
    })
}


interface Pm2Options {
    name: string;
    args: string;
}

export function startProcessLoopGetStatus (options: Pm2Options, cb: Function) {
    return new Promise(resolve => {
        startProcess({ ...options })
            .then(() => {
                let timer = startGetProcessStatusByName(options.name, (res: any[]) => {
                    const status = ((res[0] || {}).pm2_env || {}).status;
                    cb && cb(status);
                    if (status !== 'online') {
                        timer.clearLoop();
                        resolve(status)
                    }
                })
            })
    })
}

//启动pageEngine
export const startMaster = async (force: boolean): Promise<any> => {
    const processName = 'master';
    const master = await describeProcess(processName);
    if (master instanceof Error) throw master
    const masterStatus = master.filter((m: any) => ((m || {}).pm2_env || {}).status === 'online')
    if (!force && masterStatus.length === master.length && master.length !== 0) throw new Error('kungfu master 正在运行！')
    try {
        await killKfc()
    } catch (err) {
        logger.error('[TC killKfc startMaster]', err)
    }
    return startProcess({
        "name": processName,
        "args": "master"
    }, true).catch(err => logger.error('[startMaster]', err))
}

//启动ledger
export const startLedger = async (force: boolean): Promise<any> => {
    const processName = 'ledger';
    const ledger = await describeProcess(processName);
    if (ledger instanceof Error) throw ledger
    const ledgerStatus = ledger.filter((m: any): boolean => ((m || {}).pm2_env || {}).status === 'online')
    if (!force && ledgerStatus.length === ledger.length && ledger.length !== 0) throw new Error('kungfu ledger 正在运行！')
    return startProcess({
        'name': processName,
        'args': 'ledger'
    }).catch(err => logger.error('[startLedger]', err))
}


//启动md
export const startMd = (source: string): Promise<any> => {
    return startProcess({
        "name": `md_${source}`,
        "args": `md -s "${source}"`,
    }).catch(err => logger.error('[startMd]', err))
}

//启动td
export const startTd = (accountId: string): Promise<any> => {
    const { source, id } = accountId.parseSourceAccountId();
    return startProcess({
        "name": `td_${accountId}`,
        "args": `td -s "${source}" -a "${id}"`,
    }).catch(err => logger.error('[startTd]', err))
}

//启动strategy
export const startStrategy = (strategyId: string, strategyPath: string): Promise<any> => {
    strategyPath = dealSpaceInPath(strategyPath)
    const kfSystemConfig: any = readJsonSync(KF_CONFIG_PATH)
    const ifLocalPython = (kfSystemConfig.strategy || {}).python || false;
    const pythonPath = (kfSystemConfig.strategy || {}).pythonPath || '';

    if (ifLocalPython) {
        return deleteProcess(strategyId)
            .then(() => delayMiliSeconds(2000))
            .then(() => startStrategyProcess(strategyId, strategyPath, pythonPath))
    } else {
        return startProcess({
            "name": strategyId,
            "args": `strategy -n '${strategyId}' -p '${strategyPath}'`,
        }, false).catch(err => logger.error('[startStrategy]', err))
    }
}

export const startBar = (targetName: string, source: string, timeInterval: string): Promise<any> => {
    return startProcess({
        "name": targetName,
        "args": `bar -s ${source} --time-interval ${timeInterval}`
    }).catch(err => logger.error('[startBar]', err))
}

export const startCustomProcess = (targetName: string, params: string): Promise<any> => {
    return startProcess({
        "name": targetName,
        "args": `${targetName} ${params}`
    }).catch(err => logger.error(`[start${targetName}]`, err))
}

function buildProcessStatus (pList: any[]): StringToStringObject {
    let processStatus: any = {}
    Object.freeze(pList).forEach(p => {
        const name = p.name;
        const status = ((p || {}).pm2_env || {}).status
        processStatus[name] = status
    })
    return processStatus
}

function buildProcessStatusWidthDetail (pList: any[]): StringToProcessStatusDetail {
    let processStatus: any = {}
        Object.freeze(pList).forEach(p => {
            const { monit, pid, name, pm2_env } = p;
            const status = pm2_env.status;
            const created_at = pm2_env.created_at;
            const cwd = pm2_env.cwd;
            const pm_exec_path = (pm2_env.pm_exec_path || "").split('\/');
            const script = pm2_env.script || pm_exec_path[pm_exec_path.length - 1]
            const args = pm2_env.args;

            processStatus[name] = {
                status,
                monit,
                pid,
                name,
                created_at,
                script,
                cwd,
                args
            }
        })
        return processStatus
}

//列出所有进程
export const listProcessStatus = () => {
    return pm2List().then((pList: any[]): { processStatus: StringToStringObject, processStatusWithDetail: StringToProcessStatusDetail } => {
        const processStatus = buildProcessStatus(pList)
        const processStatusWithDetail = buildProcessStatusWidthDetail(pList)
        return { processStatus, processStatusWithDetail }
    })
}

export const listProcessStatusWithDetail = () => {
    return pm2List().then((pList: any[]) => buildProcessStatusWidthDetail(pList))
}

const pm2Delete = async (target: string): Promise<void> => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{ 
                pm2.delete(target, (err: any): void => {
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error('[pm2Delete]', err)
                        reject(err)
                        return;
                    }
                    resolve()
                })
            } catch (err) {
                logger.error('[TC pm2Delete]', err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

//删除进程
export const deleteProcess = (processName: string) => {
        return new Promise(async (resolve, reject) => {
            var processes = [];
            try {
                processes = await describeProcess(processName)
            } catch (err) {
                logger.error('[TC describeProcess deleteProcess]', err)
            }

            //如果進程不存在，會跳過刪除步驟
            if (!processes || !processes.length) {
                resolve(true)
                return;
            }

            pm2Delete(processName)
                .then(() => resolve(true))
                .catch(err => reject(err))
        })
}

export const stopProcess = (processName: string) => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            pm2.stop(processName, ((err: Error) => {
                if (err) {
                    reject(err)
                    return;
                }

                resolve(true)
            }))
        }).catch(err => reject(err))
    })
}

//干掉守护进程
export const killGodDaemon = () => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.killDaemon((err: any): void => {
                    pm2.disconnect()
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error(err)
                        reject(err)
                        return
                    }
                    resolve(true)
                })
            } catch (err) {
                pm2.disconnect()
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


//循环获取processStatus
export const startGetProcessStatus = (callback: Function) => {
    setTimerPromiseTask(() => {
        return listProcessStatus()
            .then(res => {
                const { processStatus, processStatusWithDetail } = res;
                if (callback) {
                    callback({
                        processStatus: Object.freeze(processStatus || {}), 
                        processStatusWithDetail: Object.freeze(processStatusWithDetail || {})
                    })
                }
            })
            .catch(err => console.error(err))
    }, 1000)
}

//循环获取processStatus
export const startGetProcessStatusByName = (name: string, callback: Function) => {
    const timer = setTimerPromiseTask(() => {
        return describeProcess(name)
            .then(res => {
                callback(res)
            })
            .catch(err => console.error(err))
    }, 1000)

    return timer
}

export const buildStartDatasetByDataSeriesIdOptions = (namespace: string, dataSeriesId: string): Pm2Options => {
    return {
        "name": namespace + dataSeriesId,
        "args": ['data', 'get', '-n', dataSeriesId, '-s', 'kfa'].join(' ')
    }
}

