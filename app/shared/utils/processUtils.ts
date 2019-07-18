import { KF_HOME, KUNGFU_ENGINE, buildProcessLogPath } from '__gConfig/pathConfig';
import { logger } from '__gUtils/logUtils';
import { platform } from '__gConfig/platformConfig';
import { getProcesses } from 'getprocesses';

const path = require('path');
const fkill = require('fkill');
const taskkill = require('taskkill');
export const pm2 = require('pm2');

//=========================== task kill =========================================

const winKill = (tasks: string[]): any => {
    let pIdList: number[] = [];
    return getProcesses().then(processes => {
        processes.forEach(p => {
            const rawCommandLine = p.rawCommandLine
            tasks.forEach((task: string): void => {
                if(rawCommandLine.indexOf(task) !== -1) pIdList.push(p.pid)
            })
        })
        if(!pIdList || !pIdList.length) return new Promise(resolve => resolve(true))
        return taskkill(pIdList, {
            force: true,
            tree: platform === 'win' 
        })
    })
}

const unixKill = (tasks: string[]): any => {
    return fkill(tasks, {
        force: true,
        silent: true 
    })
}

const kfKill = (tasks: string[]): any => {
    if(platform !== 'win') return unixKill(tasks)
    else return winKill(tasks)
}


export const killKfc = () => kfKill(['kfc'])

export const killExtra = () => kfKill(['kfc', 'pm2'])

//=========================== pm2 manager =========================================

const pm2Connect = (): Promise<void> => {
    return new Promise((resolve, reject) => {
        try{
            let noDaemon = platform === 'win' ? true : false
            if(process.env.NODE_ENV === 'development') noDaemon = false;
            pm2.connect(noDaemon, (err: Error): void => {
                if(err) {
                    process.exit(2);
                    logger.error(err);
                    pm2.disconnect();
                    reject(err);
                    return;
                }
                resolve()
            })
        }catch(err){
            pm2.disconnect()
            logger.error(err)
            reject(err)
        }  
    })
}

const pm2List = (): Promise<any[]> => {
    return new Promise((resolve, reject) => {
        try{
            pm2.list((err: Error, pList: any[]): void => {
                if(err){
                    logger.error(err)
                    reject(err)
                    return;
                }
                resolve(pList)
            })
        }catch(err){
            logger.error(err)
            reject(err)
        }
    })
}

const pm2Delete = async (target: string): Promise<void> => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.delete(target, (err: Error): void => {
                    if(err) {
                        logger.error(err)
                        reject(err)
                        return;
                    }
                    resolve()
                    
                })
            }catch(err){
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


const dealSpaceInPath = (pathname: string): string => {
    const normalizePath = path.normalize(pathname);
    return normalizePath.replace(/ /g, '\ ')
}

export const describeProcess = (name: string): Promise<any> => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.describe(name, (err: Error, res: object): void => {
                    if(err){
                        logger.error(err)
                        reject(err);
                        return;
                    }
                    resolve(res)
                })
            }catch(err){
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

export const startProcess = async (options: any, no_ext = false): Promise<object> => {
    const extensionName = platform === 'win' ? '.exe' : ''
    options = {
        ...options,
        "cwd": path.join(KUNGFU_ENGINE, 'kfc'),
        "script": `kfc${extensionName}`,
        "log_type": "json",
        "out_file": buildProcessLogPath(options.name),
        "err_file": buildProcessLogPath(options.name),
        "merge_logs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": false,
        "max_restarts": 1,
        "watch": false,
        "force": options.force === undefined ? true : options.force,
        "exec_mode" : "fork",
        "interpreterArgs": ["~harmony"],
        "env": {
            "KF_HOME": dealSpaceInPath(KF_HOME),
        }
    };

    if(no_ext) options['env']['KF_NO_EXT'] = 'on'
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.start(options, (err: Error, apps: object): void => { 
                    logger.info(err, apps)
                    if(err) {
                        logger.error(err)
                        reject(err);
                        return;
                    };
                    resolve(apps);
                })
            }catch(err){
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}

//启动pageEngine
export const startMaster = async(force: boolean): Promise<void> => {
    const processName = 'master';
    const master = await describeProcess(processName);
    if(master instanceof Error) throw master
    const masterStatus = master.filter((m: any) => m.pm2_env.status === 'online')
    if(!force && masterStatus.length === master.length && master.length !== 0) throw new Error('master正在运行！')
    try{ await killKfc() } catch (err) {}
    return startProcess({
        "name": processName,
        "args": "-l trace master"
    }, true).catch(err => logger.error(err))
}

//启动watcher
export const startWatcher = async(force: boolean): Promise<void> => {
    const processName = 'watcher';
    const watcher = await describeProcess(processName);
    if(watcher instanceof Error) throw watcher
    const watcherStatus = watcher.filter((m: any): boolean => m.pm2_env.status === 'online')
    if(!force && watcherStatus.length === watcher.length && watcher.length !== 0) throw new Error('kungfu watcher 正在运行！')
    return startProcess({
        'name': processName,
        'args': '-l trace watcher'
    }).catch(err => logger.error(err))
}

//启动md
export const startMd = (source: string): Promise<void> => {
    return startProcess({
        "name": `md_${source}`,
        "args": `md -s ${source}`,
    }).catch(err => logger.error(err))
}

//启动td
export const startTd = (accountId: string): Promise<void> => {
    const { source, id } = accountId.parseSourceAccountId();
    return startProcess({
        "name": `td_${accountId}`,
        "args": `td -s ${source} -a ${id}`,
    }).catch(err => logger.error(err))
}

//启动strategy
export const startStrategy = (strategyId: string, strategyPath: string): Promise<void> => {
    strategyPath = dealSpaceInPath(strategyPath)
    return startProcess({
        "name": strategyId,
        "args": `-l trace strategy -n ${strategyId} -p ${strategyPath}`,
    }).catch(err => logger.error(err))
}

//列出所有进程
export const listProcessStatus = () => {
    return pm2List().then((pList: any[]): StringToStringObject => {
        let processStatus: any = {}
        Object.freeze(pList).forEach(p => {
            const name = p.name;
            const status = p.pm2_env.status
            processStatus[name] = status
        })
        return processStatus
    })
}

//删除进程
export const deleteProcess = (processName: string) => {
    return new Promise(async(resolve, reject) => {
        let processes = [];
        try{
            processes = await describeProcess(processName)
        }catch(err){
            console.error(err)
        }

        //如果進程不存在，會跳過刪除步驟
        if(!processes || !processes.length) {
            resolve(true)
            return;
        }
        const pids = processes.map((prc: any): number => prc.pid);
        pm2Delete(processName)
        .then(() => resolve(true))
        .catch(err => reject(err))
        .finally(() => kfKill(pids).catch((err: Error): void => {}))
    })
}

//干掉守护进程
export const killGodDaemon = () => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.killDaemon((err: Error): void => {
                    pm2.disconnect()
                    if(err) {
                        logger.error(err)
                        reject(err)
                        return
                    }
                    resolve(true)
                })
            }catch(err){
                pm2.disconnect()
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


