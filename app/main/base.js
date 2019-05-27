const path = require('path')
const fse = require('fs-extra');
const sqlite3 = require('sqlite3').verbose();
const initGobalDB = require('__gConfig/initGlobalDB.json');
const { GLOBAL_DIR } = require('__gConfig/pathConfig');
const { logger } = require('__gUtils/logUtils');
const { platform } = require('__gConfig/platformConfig');

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!fse.existsSync(GLOBAL_DIR)){
        fse.mkdirSync(GLOBAL_DIR)
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
