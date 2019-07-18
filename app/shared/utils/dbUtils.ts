import { existsSync } from '__gUtils/fileUtils'; 

const sqlite3 = require('sqlite3').verbose();
/**
 * @param  {String} dbPath
 * @param  {sql string} sql
 * @param  {sql args} args
 */
export const runInsertUpdateDeleteDB = (dbPath: string, sql: string, args: any): Promise<any> => {
        return new Promise((resolve, reject) => {
            if(!existsSync(dbPath)){
                throw new Error(`${dbPath} 不存在！`)
            }
            const db = new sqlite3.Database(dbPath)
            db.serialize(() => {
                db.run(sql, args, (err: Error, res: any) => {
                    if(err) reject(err)
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
        const stmt = db.prepare(sql);
        batchList.forEach((l: any): void => {
            stmt.run(l)
        })
        stmt.finalize((err: Error): void => {
            if(err) reject(err)
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
        db.serialize(() => {
            db.run(`DELETE FROM ${tableName};`, (err: Error, res: any): void => {
                if(err) reject(err)
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
            if(process.env.NODE_ENV === 'development') throw new Error(`${dbPath} is not exist`)
            resolve([]);
            return;
        }
        const db = new sqlite3.Database(dbPath)
        db.serialize(() => {
            db.all(sql, args, (err: Error, res: any) => {
                if(err) reject(err)    
                else resolve(res)
                db.close();
            })
        })
    })
}




