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

export const KillKfc = () => {
    if(platform !== 'win') {
        const killList = ['kfc.exe', 'kfc']
        return fkill(killList, {
            force: true,
            tree: platform === 'win'      
        })
    }else{
        let extraList = [];
        return getProcesses().then(processes => {
            processes.forEach(p => {
                const rawCommandLine = p.rawCommandLine
                if(rawCommandLine.indexOf('kfc') !== -1) extraList.push(p)
            })
            return taskkill(extraList.map(l => l.pid), {
                force: true,
                tree: platform === 'win' 
            })
        })
    }
}

export const killExtra = () => {
    if(platform !== 'win') {
        const killList = ['pm2', 'kfc', '.pm2']
        return fkill(killList, {
            force: true,
            tree: platform === 'win'      
        })
    }else{
        let extraList = [];
        return getProcesses().then(processes => {
            processes.forEach(p => {
                const rawCommandLine = p.rawCommandLine
                if(rawCommandLine.indexOf('pm2') !== -1) extraList.push(p)
                if(rawCommandLine.indexOf('kfc') !== -1) extraList.push(p)
            })
            return taskkill(extraList.map(l => l.pid), {
                force: true,
                tree: platform === 'win' 
            })
        })
    }
}

