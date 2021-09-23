import fse from 'fs-extra';
import path from 'path';


import { KF_HOME, KFC_DIR, KF_CONFIG_PATH, APP_DIR, buildProcessLogPath } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { platform } from '@kungfu-trader/kungfu-uicc/config/platformConfig';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';
import { setTimerPromiseTask, delayMilliSeconds } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { getProcesses } from 'getprocesses';
import { getUserLocale } from 'get-user-locale';



const numCPUs = require('os').cpus() ? require('os').cpus().length : 1;
const fkill = require('fkill');
const pm2 = require('pm2');
const which = require("pm2/lib/tools/which");
const taskkill = require("taskkill");
const locale = getUserLocale().replace(/-/g, '_');

require.main = require.main || pm2;

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

export const killExtra = () => kfKill([kfc, 'pm2', 'kungfuDaemon'])


//=========================== pm2 manager =========================================

const pm2Connect = (): Promise<void> => {
    return new Promise((resolve, reject) => {
        try {
            process.env.KFC_AS_VARIANT = 'node';
            pm2.connect(false, (err: any): void => {
                if (err) {
                    err = err.length ? err[0] : err;
                    logger.error('[PM2] connect', err);
                    pm2.disconnect();
                    reject(err);
                    process.exit(2);
                }
                process.env.KFC_AS_VARIANT = '';
                resolve()
            })
        } catch (err) {
            pm2.disconnect()
            logger.error('[PM2] connect catch', err)
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
                    logger.error('[PM2] list', err)
                    reject(err)
                    return;
                }
                resolve(pList)
            })
        } catch (err) {
            logger.error('[PM2] list catch', err)
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
                    logger.error('[PM2] describeProcess', err)
                    reject(err);
                    return;
                }
                resolve(res)
            })
        } catch (err) {
            logger.error('[PM2] describeProcess catch', err)
            reject(err)
        }
    })

}

function getRocketParams(args: string, ifRocket: Boolean) {
    let rocket = ifRocket ? '-x' : '';
    
    if (numCPUs <= 4) {
        rocket = '';
    }

    if (args.includes('archive')) {
        rocket = '';
    }

    return rocket
}

function buildArgs (args: string): string {
    const kfConfig: any = fse.readJsonSync(KF_CONFIG_PATH) || {}
    const logLevel: string = ((kfConfig.log || {}).level) || '';
    const ifRocket = ((kfConfig.performance || {}).rocket) || false;
    const rocket = getRocketParams(args, ifRocket);
    return [ logLevel, args, rocket ].join(' ');
}

export const startProcess = (options: Pm2Options, no_ext = false): Promise<object> => {
    const kfcScript = platform === 'win' ? 'kfc.exe' : 'kfc';
    let optionsResolved: Pm2Options = {
        "name": options.name,
        "args": options.args, //有问题吗？
        "cwd": options.cwd || path.join(KFC_DIR),
        "script": options.script || kfcScript,
        "interpreter": options.interpreter || 'none',
        "logType": "json",
        "output": buildProcessLogPath(options.name),
        "error": buildProcessLogPath(options.name),
        "mergeLogs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": options.autorestart || false,
        "maxRestarts": options.maxRestarts || 1,
        "minUptime": 3600000, //该时间段内最大启动次数maxRestarts，如果超过则不重启，如果没超过，则一直重启
        "restartDelay": 1000,
        "watch": options.watch || false,
        "force": options.force || false,
        "execMode": "fork",
        "killTimeout": 16000,
        "env": {
            ...options.env,
            "KF_HOME": dealSpaceInPath(KF_HOME),
            "LANG": `${locale}.UTF-8`,
            "PYTHONUTF8": "1",
            "PYTHONIOENCODING": "utf8"
        },
    };

    if (no_ext) optionsResolved['env']['KF_NO_EXT'] = 'on';

    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.start(optionsResolved, (err: any, apps: object): void => {
                    if (err) {
                        console.error(err)
                        err = err.length ? err[0] : err;
                        logger.error('[PM2] startProcess', JSON.stringify(optionsResolved), err);
                        reject(err)
                        return;
                    };
                    logger.info("[PM2] startProcess", optionsResolved.name);
                    resolve(apps);
                })
            } catch (err) {
                logger.error('[PM2] startProcess catch', JSON.stringify(optionsResolved), err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

export function startProcessLoopGetStatus (options: Pm2Options, cb?: Function) {
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

export const deleteProcess = (processName: string) => {
        return new Promise(async (resolve, reject) => {
            var processes = [];
            try {
                processes = await describeProcess(processName)
            } catch (err) {
                logger.error('[PM2] deleteProcess->describeProcess catch', err)
            }

            //如果進程不存在，會跳過刪除步驟
            if (!processes || !processes.length) {
                logger.info("[PM2] deleteProcess no existed", processName)
                resolve(true)
                return;
            }

            pm2Delete(processName)
                .then(() => {
                    logger.info("[PM2] deleteProcess", processName)
                    resolve(true)
                })
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
    logger.info('[PM2] killGodDaemon')
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try {
                pm2.killDaemon((err: any): void => {
                    pm2.disconnect()
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error("[PM2] killDaemon", err)
                        reject(err)
                        return
                    }
                    resolve(true)
                })
            } catch (err) {
                pm2.disconnect()
                logger.error("[PM2] killDaemon catch", err)
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

async function pm2Delete (target: string): Promise<void> {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{ 
                pm2.delete(target, (err: any): void => {
                    if (err) {
                        err = err.length ? err[0] : err;
                        logger.error('[PM2] delete', err)
                        reject(err)
                        return;
                    }
                    resolve()
                })
            } catch (err) {
                logger.error('[PM2] delete catch', err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


// ================================= business ===================================

export const startTask = (options: Pm2Options) => {    
    return startProcess({
        script: options.script || 'index.js',
        interpreter: process.execPath,
        ...options,
        force: true
    })
}

export function startArchiveMakeTask (cb?: Function) {
    return startProcessLoopGetStatus({
        "name": "archive",
        "args": buildArgs("archive make")
    }, cb)
} 

//启动pageEngine
export const startMaster = async (force: boolean): Promise<any> => {
    const processName = 'master';
    const master = await describeProcess(processName);
    if (master instanceof Error) throw master
    const masterStatus = master.filter((m: any) => ((m || {}).pm2_env || {}).status === 'online')
    if (!force && masterStatus.length === master.length && master.length !== 0) throw new Error('kungfu master 正在运行！')
    
    try {
        await killKfc();
    } catch (err) {
        logger.error('[PM2] killKfc in startMaster catch', err)
    }

    try {
        await deleteProcess("kungfuDaemon");
    } catch (err) {
        logger.error('[PM2] kill kungfuDaemon in startMaster catch', err)
    }
    

    const args = buildArgs('service master');
    return startProcess({
        "name": processName,
        args
    }, true).catch(err => logger.error('[PM2] startProcess', processName, args, err))
}

//启动ledger
export const startLedger = async (force: boolean): Promise<any> => {
    const processName = 'ledger';
    const ledger = await describeProcess(processName);
    if (ledger instanceof Error) throw ledger
    const ledgerStatus = ledger.filter((m: any): boolean => ((m || {}).pm2_env || {}).status === 'online');
    if (!force && ledgerStatus.length === ledger.length && ledger.length !== 0) throw new Error('kungfu ledger 正在运行！');
    const args = buildArgs('service ledger');
    return startProcess({
        'name': processName,
        args
    }).catch(err => logger.error('[startLedger]', processName, args, err))
}


//启动md
export const startMd = (source: string): Promise<any> => {
    const args = buildArgs(`vendor md -s "${source}"`);
    return startProcess({
        "name": `md_${source}`,
        args,
        "maxRestarts": 3,
        "autorestart": true,
    }).catch(err => logger.error('[startMd]', `md_${source}`, args, err))
}

//启动td
export const startTd = (accountId: string): Promise<any> => {
    const { source, id } = accountId.parseSourceAccountId();
    const args = buildArgs(`vendor td -s "${source}" -a "${id}"`);
    return startProcess({
        "name": `td_${accountId}`,
        args,
        "maxRestarts": 3,
        "autorestart": true,
    }).catch(err => logger.error('[startTd]', `td_${accountId}`, args, err))
}


export const startStrategyProcess = async (name: string, strategyPath: string, pythonPath: string): Promise<object> => {
    const baseArgs = ['strategy', '-n', name, '-p', `'${strategyPath}'`].join(" ");
    const baseArgsResolved = buildArgs(baseArgs)
    const args = ['-m', 'kungfu', baseArgsResolved].join(' ')

    if (!pythonPath.trim()) {
        return Promise.reject(new Error('No local python path!'))
    }

    const fullPythonPathList = pythonPath.split('/');
    const pythonFolder = fullPythonPathList.slice(0, fullPythonPathList.length - 1).join('/')
    const pythonFile = fullPythonPathList.slice(fullPythonPathList.length - 1).join('/')

    return startProcess({
        name,
        args,
        cwd: pythonFolder,
        script: pythonFile
    })
}

//启动strategy
export const startStrategy = (strategyId: string, strategyPath: string): Promise<any> => {
    strategyPath = dealSpaceInPath(strategyPath)
    const kfSystemConfig: any = fse.readJsonSync(KF_CONFIG_PATH)
    const ifLocalPython = (kfSystemConfig.strategy || {}).python || false;
    const pythonPath = (kfSystemConfig.strategy || {}).pythonPath || '';

    if (ifLocalPython) {
        return deleteProcess(strategyId)
            .then(() => delayMilliSeconds(2000))
            .then(() => startStrategyProcess(strategyId, strategyPath, pythonPath));
    } else {
        const args = buildArgs(`strategy run -n '${strategyId}' -p '${strategyPath}'`);
        return startProcess({
            "name": strategyId,
            args,
        }).catch(err => logger.error('[PM2] startStrategy', strategyId, args, err))
    }
}

export const startBar = (targetName: string, source: string, timeInterval: string): Promise<any> => {
    return startProcess({
        "name": targetName,
        "args": buildArgs(`service bar -s ${source} --time-interval ${timeInterval}`)
    }).catch(err => logger.error('[startBar]', err))
}

export const startCustomProcess = (targetName: string, params: string): Promise<any> => {
    return startProcess({
        "name": targetName,
        "args": buildArgs(`${targetName} ${params}`)
    }).catch(err => logger.error(`[start${targetName}]`, err))
}

export const startDaemon = (): Promise<any> => {

    return startProcess({
        name: "kungfuDaemon",
        args: "",
        force: true,
        watch: process.env.NODE_ENV !== 'production',
        script: path.resolve(APP_DIR, "daemon.js"),
        interpreter: which('kfc'),
        env: {
            ...process.env,
            'KFC_AS_VARIANT': 'node'
        }
    }).catch(err => logger.error('[startDaemon]', err))
}

// =============================== function ================================
function buildProcessStatus (pList: any[]): StringToStringObject {
    let processStatus: any = {}
    Object.freeze(pList).forEach(p => {
        const name = p.name;
        const status = ((p || {}).pm2_env || {}).status
        processStatus[name] = status
    })
    return processStatus
}

export function buildProcessStatusWidthDetail (pList: any[]): StringToProcessStatusDetail {
    let processStatus: StringToProcessStatusDetail = {};
    Object.freeze(pList).forEach(p => {
        const { monit, pid, name, pm2_env, pm_id } = p;
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
            pm_id,
            name,
            created_at,
            script,
            cwd,
            args
        }
    })

    return processStatus
}

//循环获取processStatus
function startGetProcessStatusByName (name: string, callback: Function) {
    const timer = setTimerPromiseTask(() => {
        return describeProcess(name)
            .then(res => {
                callback(res)
            })
            .catch(err => console.error(err))
    }, 1000)

    return timer
}

export const sendDataToProcessIdByPm2 = (topic: string, pm2Id: number, processName: string, data: any): void => {
    pm2.sendDataToProcessId({
        type: 'process:msg',
        data,
        id: pm2Id,
        topic: topic
    }, (err: Error) => {
        if (err) {
            console.error(processName, err)
        }
    })
}

export const sendDataToDaemonByPm2 = (topic: string, data: any): Promise<any> => {
    return getKungfuDaemonPmId()
        .then(pmid => {
            if (pmid === -1) {
                return Promise.reject('KungfuDaemon not exsited！')
            } else {
                return pmid
            }
        })
        .then((pmId: number): void => {
            if (!pmId) return;
            sendDataToProcessIdByPm2(topic, pmId, "kungfuDaemon", data)
        })
}

function getKungfuDaemonPmId (): Promise<any> {
    return listProcessStatus()
        .then(({ processStatusWithDetail }) => {
            const kungfuDaemonPrc: ProcessStatusDetail = processStatusWithDetail['kungfuDaemon'] || {};
            return kungfuDaemonPrc.pm_id || -1;
        })
}


