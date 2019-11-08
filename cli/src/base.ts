const path = require('path');
const colors = require('colors');

declare global {
    interface Console {
        success: Function
    }
}

console.success = (log: string) => {
    console.log(`${colors.green('SUCCESS')} ${log}`)
}

console.error = (log: string) => {
    console.log(`${colors.red('ERROR')} ${log}`)
}

if (process.env.NODE_ENV === 'production') {
    //@ts-ignore
    process.resourcesPath =  path.join(path.dirname(process.execPath), '..').replace(/\\/g, '\\\\')
}

const initGlobalDB = require('__gConfig/initGlobalDB.json')
const { BASE_DB_DIR, DEFUALT_DB_DIR } = require('__gConfig/pathConfig');
const { logger } = require('__gUtils/logUtils');
const { existsSync, addFile } = require('__gUtils/fileUtils');

const fse = require('fs-extra');
const sqlite3 = require('kungfu-core').sqlite3.verbose();

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!existsSync(BASE_DB_DIR)) addFile('', BASE_DB_DIR, 'folder');

    //循环建立表
    Object.keys(initGlobalDB).forEach((dbName: string) => {
        const db = new sqlite3.Database(path.join(BASE_DB_DIR, `${dbName}.db`));
        const tables = initGlobalDB[dbName];
        db.serialize(() => {
            tables.forEach((table: any) => {
                db.run(table.sql)
            })
        })	
        db.close();
    })

    //commission.db
    fse.copy(
        path.join(DEFUALT_DB_DIR, 'commission.db'), 
        path.join(BASE_DB_DIR, 'commission.db')
    )
    .catch((err: Error): void => {
        if(err) logger.error(err);
    })

    //holidays.db
    fse.copy(
        path.join(DEFUALT_DB_DIR, 'holidays.db'), 
        path.join(BASE_DB_DIR, 'holidays.db')
    )
    .catch((err: Error): void => {
        if(err) logger.error(err);
    })
}
