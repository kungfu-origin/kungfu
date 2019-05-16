const path = require('path')
const sqlite3 = require('sqlite3').verbose();
const fs = require('fs');
const fse = require('fs-extra');
const initGobalDB = require('__gConfig/initGlobalDB.json');
const {GLOBAL_DIR} = require('__gConfig/pathConfig');
const {logger} = require('__gUtils/logUtils');
const {platform} = require('__gConfig/platformConfig');
const fkill = require('fkill');
const {getProcesses} = require('getprocesses');
const taskkill = require('taskkill')

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!fs.existsSync(GLOBAL_DIR)){
        fs.mkdirSync(GLOBAL_DIR)
    }

    //循环建立表
    Object.keys(initGobalDB).forEach((dbName) => {
        const db = new sqlite3.Database(path.join(GLOBAL_DIR, `${dbName}.db`));
        const tables = initGobalDB[dbName];
        db.serialize(() => {
            tables.forEach((table) => {
                db.run(table.sql)
            })
        })	
        db.close();
    })

    //commission.db
    fse.copy(path.join(__resources, 'default', 'commission.db'), path.join(GLOBAL_DIR, 'commission.db'), err => {
        if(err) logger.error(err);
    })

    //holidays.db
    fse.copy(path.join(__resources, 'default', 'holidays.db'), path.join(GLOBAL_DIR, 'holidays.db'), err => {
        if(err) logger.error(err);
    })
}

const winKill = (tasks) => {
    let pIdList = [];
    return getProcesses().then(processes => {
        processes.forEach(p => {
            const rawCommandLine = p.rawCommandLine
            tasks.forEach(task => {
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

const unixKill = (tasks) => {
    return fkill(tasks, {
        force: true,
        tree: platform === 'win'      
    })
}

const kfKill = (tasks) => {
    if(platform !== 'win') return unixKill(tasks)
    else return winKill(tasks)
}


export const KillKfc = () => kfKill(['kfc'])

export const killExtra = () => kfKill(['kfc', 'pm2'])

