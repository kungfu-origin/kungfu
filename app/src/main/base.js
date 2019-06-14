import initGlobalDB from '__gConfig/initGlobalDB.json'
import { GLOBAL_DIR } from '__gConfig/pathConfig';
import { logger } from '__gUtils/logUtils'
const path = require('path')
const fse = require('fs-extra');
const sqlite3 = require('sqlite3').verbose();

;if (process.env.NODE_ENV === 'production') {
    if(process.env.APP_TYPE === 'cli') global.__resources = path.join('.', 'resources').replace(/\\/g, '\\\\')// eslint-disable-line{{/if_eq}}
    else global.__resources = path.join(__dirname, '/resources').replace(/\\/g, '\\\\')// eslint-disable-line{{/if_eq}}
}

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!fse.existsSync(GLOBAL_DIR)){
        fse.mkdirSync(GLOBAL_DIR)
    }

    //循环建立表
    Object.keys(initGlobalDB).forEach((dbName) => {
        const db = new sqlite3.Database(path.join(GLOBAL_DIR, `${dbName}.db`));
        const tables = initGlobalDB[dbName];
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
