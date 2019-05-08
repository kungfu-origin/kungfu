const path = require('path');
const {BASE_DIR, KUNGFU_ENGINE, buildProcessLogPath} = require('__gConfig/pathConfig');
const {logger} = require('__gUtils/logUtils');
const {platform} = require('__gConfig/platformConfig');
const fkill = require('fkill');
export const pm2 = require('pm2')


const pm2Connect = () => {
    return new Promise((resolve, reject) => {
        try{
            pm2.connect((err) => {
                if(err) {
                    process.exit(2);
                    logger.error(err);
                    pm2.disconnect();
                    reject(err);
                    return;
                }
                resolve([])
            })
        }catch(err){
            pm2.disconnect()
            logger.error(err)
            reject(err)
        }  
    })
}

const pm2List = () => {
    return new Promise((resolve, reject) => {
        try{
            pm2.list((err, pList) => {
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

const pm2Delete = (target) => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.delete(target, err => {
                    if(err) {
                        logger.error(err)
                        reject(err)
                        return;
                    }
                    resolve(true)
                })
            }catch(err){
                logger.error(err)
                reject(err)
            }
        }).catch(err => reject(err))
    })
}


const dealSpaceInPath = (pathname) => {
    if (platform === 'win') return pathname
    return eval('"' + pathname.replace(/ /g, '\\ ') + '"')
}
// .replace(/ /g, '\\ ')
// .replace(/\\/g, '\\\\').replace(/ /g, '\\ ')
export const describeProcess = (name) => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.describe(name, (err, res) => {
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

export const startProcess = async (options) => {
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
        "env": {
            // "PM2_HOME": PM2_DIR,
            "KF_HOME": dealSpaceInPath(BASE_DIR),
            // "ELECTRON_RUN_AS_NODE": true,
            NODE_ENV: "production",
        }
    }

    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.start(options, (err, apps) => { 
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
export const startPageEngine = async(force) => {
    const processName = 'page_engine'
    const pageEngines = await describeProcess(processName);
    if(pageEngines instanceof Error) throw pageEngines
    if(!force && pageEngines.length) throw new Error('page_engine正在运行！')
    return startProcess({
        "name": processName,
        "args": "paged --name paged",
    }).catch(err => logger.error(err))
}

//启动交易日服务
export const startCalendarEngine = async(force) => {
    const processName = 'calendar_engine'
    const calendarEngines = await describeProcess(processName);
    if(calendarEngines instanceof Error) throw calendarEngines
    if(!force && calendarEngines.length) throw new Error('calendar_engine正在运行！')
    return startProcess({
        "name": "calendar_engine",
        "args": "calendar --name calendar",
    }).catch(err => logger.error(err))    
}

//启动md
export const startMd = (resource, processName) => {
    return startProcess({
        "name": processName,
        "args": `md_${resource}`,
    }).catch(err => logger.error(err))      
}

//启动td
export const startTd = (resource, processName) => {
    return startProcess({
        "name": processName,
        "args": `td_${resource} --name ${processName}`,
    })   
}


//启动strategy
export const startStrategy = (strategyId, strategyPath) => {
    strategyPath = dealSpaceInPath(strategyPath)
    return startProcess({
        "name": strategyId,
        "args": `strategy --name ${strategyId} --path ${strategyPath}`,
    }).catch(err => {
        logger.error('startStrategy-err', err)
    })   
}


//列出所有进程
export const listProcessStatus = () => {
    return new Promise((resolve, reject) => {
        pm2List().then(pList => {
            let processStatus = {}
            Object.freeze(pList).forEach(p => {
                const name = p.name;
                const status = p.pm2_env.status
                processStatus[name] = status
            })
            resolve(processStatus)
        }).catch(err => reject(err))
    })
}

//删除进程
export const deleteProcess = async(processName) => {
    let processes;
    try{
        processes = await describeProcess(processName)
    }catch(err){
        console.error(err)
    }
    const pids = processes.map(prc => prc.pid);
    fkill(pids).catch(err => console.error(err))
    return pm2Delete(processName)
}


//删除所有进程
export const killAllProcess = async() => {
    const processes = await pm2List();
    const pids = processes.map(p => p.pid).filter(p => !!p)
    return fkill(pids, {
        force: true,
        ignoreCase: true
    })
}


//干掉守护进程
export const killGodDaemon = () => {
    return new Promise((resolve, reject) => {
        pm2Connect().then(() => {
            try{
                pm2.killDaemon(err => {
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
            pm2.disconnect()
        }).catch(err => reject(err))
    })
}