
import { copySync, existsSync } from '__gUtils/fileUtils';
import { GLOBAL_COMMISSION_DB } from '__gConfig/pathConfig';
import { runSelectDB, runBatchInsertDB, runClearDB } from '__gUtils/dbUtils';

export const getFeeSettingData = () => {
    return runSelectDB(GLOBAL_COMMISSION_DB,`SELECT rowid, * FROM commission`)
}

export const setFeeSettingData = (feeSettingData: any, ) => {
    if(feeSettingData.length < 1) throw new Error('fees length is 0')
    if(!existsSync(GLOBAL_COMMISSION_DB)) throw new Error('commission.db is not exist!')
    return new Promise((resolve, reject) => {
        const keys = Object.keys(feeSettingData[0] || {})
        const q = [...keys].fill("?")
        runClearDB(GLOBAL_COMMISSION_DB, 'commission')
        .then(() => runBatchInsertDB(GLOBAL_COMMISSION_DB, `INSERT INTO commission(${keys.join(", ")}) VALUES (${q.join(", ")})`, feeSettingData.map((f: any): any[] => [...Object.values(f)])))
        .then(() => resolve(true))
        .catch((err: Error): void => reject(err))
    })
}
