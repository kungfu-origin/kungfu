import initGlobalDB from '__gConfig/initGlobalDB.json'
import { BASE_DB_DIR, DEFUALT_DB_DIR, KF_CONFIG_DEFAULT_PATH, KF_CONFIG_PATH, KF_TARADING_CONFIG_DEFAULT_PATH, KF_TARADING_CONFIG_PATH, KF_HOME } from '__gConfig/pathConfig';
import { logger } from '__gUtils/logUtils'
import { existsSync, addFileSync, readJsonSync, outputJsonSync } from '__gUtils/fileUtils';

const path = require('path');
const fse = require('fs-extra');
const sqlite3 = require('kungfu-core').sqlite3.verbose();
const kungfu = require('kungfu-core').kungfu;

interface SqlTable {
    table: string;
    sql: string;
};

export const locator = kungfu.locator(KF_HOME);
export const watcher = kungfu.watcher(locator);

console.log('kungfu setup');
watcher.setup();
watcher.step();
console.log(watcher.ledger);
console.log('kungfu done');

export const initDB = () => {
    //检测是否有数据库目录，没有则创建
    if(!existsSync(BASE_DB_DIR)) addFileSync('', BASE_DB_DIR, 'folder');

    //循环建立表
    Object.keys(initGlobalDB).forEach((dbName: string) => {
        const db = new sqlite3.Database(path.join(BASE_DB_DIR, `${dbName}.db`));
        const tables = initGlobalDB[dbName];
        db.parallelize(() => {
            tables.forEach((table: SqlTable) => {
                db.run(table.sql)
            })
        });
        db.close();
    });

    //commission.db
    fse.copy(
        path.join(DEFUALT_DB_DIR, 'commission.db'), 
        path.join(BASE_DB_DIR, 'commission.db')
    )
    .catch((err: Error) => {
        if(err) logger.error(err);
    });

    //holidays.db
    fse.copy(
        path.join(DEFUALT_DB_DIR, 'holidays.db'), 
        path.join(BASE_DB_DIR, 'holidays.db')
    )
    .catch((err: Error) => {
        if(err) logger.error(err);
    })
};


export const initConfig = () => {
    if(!existsSync(KF_CONFIG_PATH)) {
        addFileSync('', KF_CONFIG_PATH, 'file');
        const kfConfigJSON = readJsonSync(KF_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_CONFIG_PATH, kfConfigJSON)
    }

    if(!existsSync(KF_TARADING_CONFIG_PATH)) {
        addFileSync('', KF_TARADING_CONFIG_PATH, 'file');
        const kfTradingConfigJSON = readJsonSync(KF_TARADING_CONFIG_DEFAULT_PATH);
        outputJsonSync(KF_TARADING_CONFIG_PATH, kfTradingConfigJSON)
    }
};