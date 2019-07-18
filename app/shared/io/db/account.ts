import { buildDateRange } from '__gUtils/busiUtils';
import { runSelectDB, runBatchInsertDB, runInsertUpdateDeleteDB, runClearDB } from '__gUtils/dbUtils';
import { ACCOUNTS_DB, LIVE_TRADING_DATA_DB, GLOBAL_COMMISSION_DB, buildAccountCommissionDBPath } from '__gConfig/pathConfig';
import { copySync, existsSync } from '__gUtils/fileUtils';
import moment from "moment"

/**
 * 获取账户列表
 */
export const getAccountList = () => {
    return runSelectDB(ACCOUNTS_DB, 'SELECT * FROM account_config ORDER BY account_id')
}

export const getAccountBySource = (sourceName: string) => {
    return runSelectDB(ACCOUNTS_DB, 'SELECT * FROM account_config WHERE source_name = ?', sourceName)
}

/**
 * 新建账户
 * @param {String} account_id 账户id
 */

export const addAccount = (account_id: string, source_name: string, receive_md: boolean, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'INSERT INTO account_config(account_id, source_name,receive_md, config) VALUES (?, ?, ?, ?)', [account_id, source_name, receive_md, config])
}

/**
 * 更改账户配置
 * @param  {} account_id
 */
export const updateAccountConfig = (account_id: string, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE account_config SET config = ? WHERE account_id = ?', [config, account_id])
}

/**
 * 删除账户
 * @param  {} account_id 账户id
 */
export const deleteAccount = (account_id: string) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, "DELETE FROM account_config WHERE account_id = ?", account_id)
}

/**
 * 改变账户行情
 * @param  {} account_id 账户id
 * @param  {} receive_md 是否接受行情
 */
export const changeAccountMd = (account_id: string, receive_md: boolean) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE account_config SET receive_md = ? WHERE account_id = ?', [receive_md, account_id])
}

/**
 * 获取账户资金情况
 * 
 */
export const getAccountAsset = () => {
    return runSelectDB(LIVE_TRADING_DATA_DB, 'SELECT * FROM account')
}

/**
 * 获取账户持仓情况
 * 
 */
export const getAccountPos = (accountId: string, { instrumentId, type }: TradingDataFilter) => {
    accountId = accountId.toAccountId();
    instrumentId = instrumentId || '';
    type = type || '';
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM account_position` + 
        ` where account_id = "${accountId}"` + 
        ` AND instrument_id LIKE '%${instrumentId}%'` +
        (type ? ` AND instrument_type = ${type || ''}` : ``) + 
        ' ORDER BY instrument_id'
    )
}

/**
 * 获取账户委托情况
 * @param {String} accountId  账户id
 * @param {String} id  模糊查询的id部分数据
 * @param {Array} dateRange  时间查询的开始时间和结束时间
 */
export const getAccountOrder = (accountId: string, { id, dateRange }: TradingDataFilter, tradingDay: string) => {
    accountId = accountId.toAccountId();    
    id = id || '';
    dateRange = dateRange || [];
    const filterDate = buildDateRange(dateRange, tradingDay)
    //查询总数的时候也需要根据筛选条件来
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM orders` +  
        ` WHERE account_id='${accountId}'` +
        ` AND (order_id LIKE '%${id}%'` +
        ` OR instrument_id LIKE '%${id}%'` +
        ` OR client_id LIKE '%${id}%')` +
        ` AND insert_time >= ${filterDate[0]}` +
        ` AND insert_time < ${filterDate[1]}` +
        (dateRange.length ? `` : ` AND status NOT IN (3,4,5,6)`) + //有日期筛选的时候,获取所有状态的数据；无日期的时候，获取的是当天的且未完成的
        ` ORDER BY order_id DESC`
    )
}

/**
 * 获取账户成交情况
 * 
 */
export const getAccountTrade = (accountId: string, { id, dateRange }: TradingDataFilter, tradingDay: string) => {
    accountId = accountId.toAccountId();
    id = id || '';
    dateRange = dateRange || [];
    const filterDate = buildDateRange(dateRange, tradingDay)
    //查询总数的时候也需要根据筛选条件来
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT rowId, * FROM trades` + 
        ` WHERE account_id="${accountId}"` +
        ` AND (instrument_id LIKE '%${id}%'` +
        ` OR client_id LIKE '%${id}%')` +
        ` AND trade_time > ${filterDate[0]}` + 
        ` AND trade_time < ${filterDate[1]}` + //有日期筛选的时候
        ` ORDER BY trade_id DESC`
    )
}


/**
 * 获取账户收益曲线分钟线
 * 
 */
export const getAccountPnlMin = (accountId: string, tradingDay: string) => {
    if(!tradingDay) throw new Error('无交易日！')
    if(!accountId) return new Promise(resolve => resolve([]))
    accountId = accountId.toAccountId();
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM account_snapshot` + 
        ` WHERE trading_day = '${tradingDay}'` +
        ` AND account_id='${accountId}'`
    )
}

/**
 * 获取账户收益曲线日线
 * 
 */
export const getAccountPnlDay = (accountId: string) => {
    if(!accountId) return new Promise(resolve => resolve([]))
    accountId = accountId.toAccountId();
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        'SELECT * FROM (select * from account_snapshot ORDER BY update_time DESC)' + 
        ` WHERE account_id='${accountId}'` +
        ` GROUP BY trading_day`
    )
}


export const setFeeSettingData = (accountId: string, feeSettingData: any) => {
    if(feeSettingData.length < 1) throw new Error('fees length is 0')
    const COMMISSION_DB = buildAccountCommissionDBPath(accountId)
    if(!existsSync(COMMISSION_DB)) throw new Error('commission.db is not exist!')
    return new Promise((resolve, reject) => {
        const keys = Object.keys(feeSettingData[0] || {})
        const q = [...keys].fill("?")
        runClearDB(COMMISSION_DB, 'commission')
        .then(() => runBatchInsertDB(COMMISSION_DB, `INSERT INTO commission(${keys.join(", ")}) VALUES (${q.join(", ")})`, feeSettingData.map((f: any): any[] => [...Object.values(f)])))
        .then(() => resolve(true))
        .catch((err: Error): void => reject(err))
    })
}

export const getFeeSettingData = (accountId: string) => {
    const COMMISSION_DB = buildAccountCommissionDBPath(accountId)
    if(!existsSync(COMMISSION_DB)) {
        try{
            copySync(GLOBAL_COMMISSION_DB, COMMISSION_DB)
        }catch(err){
            throw err
        }
    }
    return runSelectDB(COMMISSION_DB,`SELECT * FROM commission`)
}

