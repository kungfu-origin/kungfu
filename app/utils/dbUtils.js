const sqlite3 = require('sqlite3').verbose();
const { existsSync } = require('__gUtils/fileUtils'); 

/**
 * @param  {String} dbPath
 * @param  {sql string} sql
 * @param  {sql args} args
 */
export const runInsertUpdateDeleteDB = (dbPath, sql, args) => {
        return new Promise((resolve, reject) => {
            if(!existsSync(dbPath)){
                throw new Error(`${dbPath} 不存在！`)
            }
            const db = new sqlite3.Database(dbPath)
            db.serialize(() => {
                db.run(sql, args, (err, res) => {
                    if(err) reject(err)
                    else resolve(res)
                    db.close();
                })
            })
        })
    }

export const runBatchInsertDB = (dbPath, sql, batchList) => {
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            throw new Error(`${dbPath} 不存在！`)
        }
        const db = new sqlite3.Database(dbPath)
        const stmt = db.prepare(sql);
        batchList.forEach(l => {
            stmt.run(l)
        })
        stmt.finalize((err) => {
            if(err) reject(err)
            else resolve(true)
            db.close();
        })
    })
}
    
/**
 * @param  {String} dbPath
 * @param  {String} tableName
 */
export const runClearDB = (dbPath, tableName) => {
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            throw new Error(`${dbPath} 不存在！`)
        }
        const db = new sqlite3.Database(dbPath)
        db.serialize(() => {
            db.run(`DELETE FROM ${tableName};`, (err, res) => {
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
export const runSelectDB = (dbPath, sql, args) =>{
    return new Promise((resolve, reject) => {
        if(!existsSync(dbPath)){
            resolve([]);
            return;
        }
        const db = new sqlite3.Database(dbPath)
        db.serialize(() => {
            db.all(sql, args, (err, res) => {
                if(err) reject(err)    
                else resolve(res)
                db.close();
            })
        })
    })
}




