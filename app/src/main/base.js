import initGlobalDB from '__gConfig/initGlobalDB.json'
import { BASE_DB_DIR, DEFUALT_DB_PATH } from '__gConfig/pathConfig';
import { logger } from '__gUtils/logUtils'
import { existsSync, addFile } from '__gUtils/fileUtils';
const path = require('path')
const fse = require('fs-extra');
const sqlite3 = require('kungfu-core').sqlite3.verbose();

if (process.env.NODE_ENV !== 'development') {
    global.__resources = path.join(__dirname, '/resources').replace(/\\/g, '\\\\')
}

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!existsSync(BASE_DB_DIR)) addFile('', BASE_DB_DIR, 'folder');

    //循环建立表
    Object.keys(initGlobalDB).forEach((dbName) => {
        const db = new sqlite3.Database(path.join(BASE_DB_DIR, `${dbName}.db`));
        const tables = initGlobalDB[dbName];
        db.serialize(() => {
            tables.forEach((table) => {
                db.run(table.sql)
            })
        })	
        db.close();
    })

    //commission.db
    fse.copy(
        path.join(DEFUALT_DB_PATH, 'commission.db'), 
        path.join(BASE_DB_DIR, 'commission.db')
    )
    .catch(err => {
        if(err) logger.error(err);
    })

    //holidays.db
    fse.copy(
        path.join(DEFUALT_DB_PATH, 'holidays.db'), 
        path.join(BASE_DB_DIR, 'holidays.db')
    )
    .catch(err => {
        if(err) logger.error(err);
    })
}
