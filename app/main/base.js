const path = require('path')
const sqlite3 = require('sqlite3').verbose();
const fs = require('fs');
const fse = require('fs-extra');
const initGobalDB = require('__gConfig/initGlobalDB.json');
const {GLOBAL_DIR} = require('__gConfig/pathConfig');
const {logger} = require('__gUtils/logUtils');
const fkill = require('fkill');
const {platform} = require('__gConfig/platformConfig');


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

}


export const killAll = () => {
    const killList = platform === 'win' ? ['kfc.exe'] : ['kfc', 'pm2']
    return fkill(killList, {
        force: true,
        tree: true,
        ignoreCase: true
    })
}