const sqlite3 = require('sqlite3').verbose();
const fs = require('fs');


    /**
     * @param  {String} dbPath
     * @param  {sql string} sql
     * @param  {sql args} args
     */
export const runInsertUpdateDeleteDB = (dbPath, sql, args) => {
        
        return new Promise((resolve, reject) => {
            if(!fs.existsSync(dbPath)){
                resolve([])
                return new Error(`${dbPath} 不存在！`)
            }
            const db = new sqlite3.Database(dbPath)
            db.serialize(() => {
                db.run(sql, args, (err, res) => {
                    if(!err){
                        resolve(res)
                    }else{
                        reject(err)
                    }
                })
            })
            db.close();
        })
    }
    
    /**
     * @param  {String} dbPath
     * @param  {sql string} sql
     * @param  {sql args} args
     */
export const runSelectDB = (dbPath, sql, args) =>{
    return new Promise((resolve, reject) => {
        if(!fs.existsSync(dbPath)){
            resolve([])
            return new Error(`${dbPath} 不存在！`)
        }
        const db = new sqlite3.Database(dbPath)
        db.serialize(() => {
            db.all(sql, args, (err, res) => {
                if(!err) resolve(res)    
                else {
                    console.error(err)
                    resolve([])
                }
                db.close();
            })
        })
    })
}




