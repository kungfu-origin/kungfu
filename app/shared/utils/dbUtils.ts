import { existsSync } from '__gUtils/fileUtils';
import { logger } from '__gUtils/logUtils';
import { KF_HOME } from '__gConfig/pathConfig';

const sqlite3 = require('kungfu-core').sqlite3.verbose();
const kungfu = require('kungfu-core').kungfu;

const locator = kungfu.locator(KF_HOME);
export const renderWatcher = kungfu.watcher(locator, 'watcher_render');
renderWatcher.locator = locator;

/**
 * @param  {String} dbPath
 * @param  {sql string} sql
 * @param  {sql args} args
 */
export const runInsertUpdateDeleteDB = (dbPath: string, sql: string, args?: any): Promise<any> => {   
    return new Promise((resolve, reject) => {
            if(!existsSync(dbPath)){
                throw new Error(`${dbPath} 不存在！`)
            }
            const db = new sqlite3.Database(dbPath)
            db.configure('busyTimeout', 10000)
            db.serialize(() => {
                db.run(sql, args, (err: Error, res: any) => {
                    if(err) {
                        logger.error('runInsertUpdateDeleteDB', dbPath, sql, args, err);
                        reject(err)
                    }
                    else resolve(res)
                    db.close();
                })
            })
        })
    }

export const runBatchInsertDB = (dbPath: string, sql: string, batchList: any[]): Promise<void> => {
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            throw new Error(`${dbPath} 不存在！`)
        }
        const db = new sqlite3.Database(dbPath)
        db.configure('busyTimeout', 10000)
        const stmt = db.prepare(sql);
        batchList.forEach((l: any): void => {
            stmt.run(l)
        })
        stmt.finalize((err: Error): void => {
            if(err) {
                logger.error('runBatchInsertDB', dbPath, sql, batchList, err);
                reject(err)
            }
            else resolve()
            db.close();
        })
    })
}
    
/**
 * @param  {String} dbPath
 * @param  {String} tableName
 */
export const runClearDB = (dbPath: string, tableName: string): Promise<any> => {
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            throw new Error(`${dbPath} 不存在！`)
        }
        const db = new sqlite3.Database(dbPath)
        db.configure('busyTimeout', 10000)
        db.serialize(() => {
            db.run(`DELETE FROM ${tableName};`, (err: Error, res: any): void => {
                if(err) {
                    logger.error('runClearDB', dbPath, tableName, err);
                    reject(err)
                }
                else resolve(res)
                db.close();
            })
        })        
    })
}


    
/**
 * @param  {String} dbPath
 * @param  {sql string} sql
 * @param  {sql args} args
 */
export const runSelectDB = (dbPath: string, sql: string, args?: any): Promise<any[]> =>{
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            if(process.env.NODE_ENV !== 'production') throw new Error(`${dbPath} is not exist`)
            resolve([]);
            return;
        }
        const db = new sqlite3.Database(dbPath)
        db.configure('busyTimeout', 10000)
        db.serialize(() => {
            db.all(sql, args, (err: Error, res: any) => {
                if(err) {
                    logger.error('runSelectDB', dbPath, sql, args, err);
                    reject(err)
                } 
                else resolve(res)
                db.close();
            })
        })
    })
}




