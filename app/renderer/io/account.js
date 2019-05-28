import {runSelectDB, runBatchInsertDB, runInsertUpdateDeleteDB, runClearDB} from '__gUtils/dbUtils';
import {ACCOUNTS_DB, buildGloablCommissionDBPath, buildAccountCommissionDBPath, buildGateWayStateDBPath, buildAccountAssetsDBPath,buildAccountTradesDBPath, buildAccountOrdersDBPath, buildAccountSnapshortsDBPath} from '__gConfig/pathConfig';
import { copySync, existsSync } from '__gUtils/fileUtils';
import moment from "moment"

/**
 * 获取账户列表
 */
export const getAccountList = () => {
    return runSelectDB(ACCOUNTS_DB, 'SELECT * FROM account_config ORDER BY account_id')
}

/**
 * 新建账户
 * @param {String} account_id 账户id
 */

export const addAccount = (account_id, source_name, receive_md, config) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'INSERT INTO account_config(account_id, source_name,receive_md, config) VALUES (?, ?, ?, ?)', [account_id, source_name, receive_md, config])
}

/**
 * 更改账户配置
 * @param  {} account_id
 */
export const updateAccountConfig = (account_id, config) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE account_config SET config = ? WHERE account_id = ?', [config, account_id])
}

/**
 * 删除账户
 * @param  {} account_id 账户id
 */
export const deleteAccount = (account_id) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, "DELETE FROM account_config WHERE account_id = ?", account_id)
}


/**
 * 改变账户行情
 * @param  {} account_id 账户id
 * @param  {} receive_md 是否接受行情
 */
export const changeAccountMd = (account_id, receive_md) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE account_config SET receive_md = ? WHERE account_id = ?', [receive_md, account_id])
}


/**
 * 获取md/td状态列表
 * 
 */
export const getMdTdState = (gatewayName) => {
    return runSelectDB(buildGateWayStateDBPath(gatewayName), 'SELECT * FROM gateway_state')
}

/**
 * 获取账户资金情况
 * 
 */
export const getAccountAsset = (accountId) => {
    return runSelectDB(buildAccountAssetsDBPath(accountId), 'SELECT * FROM account')
}

/**
 * 获取账户持仓情况
 * 
 */
export const getAccountPos = (accountId, {instrumentId, type}) => {
    return runSelectDB(buildAccountAssetsDBPath(accountId), (`SELECT * FROM position where instrument_id LIKE '%${instrumentId || ''}%'` + (type ? ` AND instrument_type = ${type || ''}` : ``) + ' ORDER BY instrument_id'))
}
/**
 * 获取账户成交情况
 * 
 */
export const getAccountTrade = (accountId, {id, dateRange}, tradingDay) => {
    const momentDay = tradingDay ? moment(tradingDay) : moment();
    //日期控件选出的日期都是0点的，需要加上一天才能将最后一天包含在内
    const startDate = (moment(momentDay.format('YYYY-MM-DD')).valueOf()) * 1000000
    const endDate = (moment(momentDay.add(1,'d').format('YYYY-MM-DD')).valueOf()) * 1000000
    const filterDate = dateRange ? [moment(dateRange[0]).valueOf() * 1000000, (moment(dateRange[1]).add(1,'d').valueOf() * 1000000)] : [startDate, endDate]
    return new Promise((resolve, reject) => {
        //查询总数的时候也需要根据筛选条件来
        const sql = `WHERE (instrument_id LIKE '%${id}%' OR client_id LIKE '%${id}%')` + //有id筛选的时候
        (` AND trade_time > ${filterDate[0]} AND trade_time < ${filterDate[1]}`) //有日期筛选的时候
        runSelectDB(buildAccountTradesDBPath(accountId), `SELECT rowId, * FROM trade ${sql} ORDER BY id DESC`).then(trade => {
            resolve({data: trade})
        }).catch(err => {
            reject(err)
        })
    })
}

/**
 * 获取账户委托情况
 * @param {String} accountId  账户id
 * @param {String} id  模糊查询的id部分数据
 * @param {Array} dateRange  时间查询的开始时间和结束时间
 */
export const getAccountOrder = (accountId, {id, dateRange}, tradingDay) => {
    const momentDay = tradingDay ? moment(tradingDay) : moment();
    //获取当天是日期范围
    const startDate = (moment(momentDay.format('YYYY-MM-DD')).valueOf()) * 1000000
    const endDate = (moment(momentDay.add(1,'d').format('YYYY-MM-DD')).valueOf()) * 1000000
    //日期控件选出的日期都是0点的，需要加上一天才能将最后一天包含在内
    const filterDate = dateRange ? [moment(dateRange[0]).valueOf() * 1000000, (moment(dateRange[1]).add(1,'d').valueOf() * 1000000)] : [startDate, endDate]
    return new Promise((resolve, reject) => {
        //查询总数的时候也需要根据筛选条件来
        const sql = `WHERE (order_id LIKE '%${id || ''}%' OR instrument_id LIKE '%${id || ''}%' OR client_id LIKE '%${id || ''}%')` + //有id筛选的时候
        ` AND insert_time >= ${filterDate[0]} AND insert_time < ${filterDate[1]}` + 
        (dateRange ? `` : ` AND status NOT IN (3,4,5,6)`) //有日期筛选的时候,获取所有状态的数据；无日期的时候，获取的是当天的且未完成的
        runSelectDB(buildAccountOrdersDBPath(accountId), `SELECT * FROM orders ${sql} ORDER BY order_id DESC`).then(orders => {
            resolve(orders)
        }).catch(err => {
            reject(err)
        })
    })  
}

/**
 * 获取委托总行数
 * 
 */
export const getOrderCount = (accountId, sql) => {
    return runSelectDB(buildAccountOrdersDBPath(accountId),`SELECT COUNT(*) FROM orders ${sql}`)
}

/**
 * 获取账户收益曲线分钟线
 * 
 */
export const getAccountPnlMin = (accountId, tradingDay) => {
    if(!tradingDay) return new Promise((resolve, reject) => {reject(new Error('无交易日！'))})
    return runSelectDB(buildAccountSnapshortsDBPath(accountId), 
    `SELECT * FROM trading_account_1m_snapshots WHERE trading_day = '${tradingDay}'`)
}

/**
 * 获取账户收益曲线日线
 * 
 */
export const getAccountPnlDay = (accountId) => {
    return runSelectDB(buildAccountSnapshortsDBPath(accountId), 'SELECT * FROM trading_account_1d_snapshots')
}


export const setFeeSettingData = (accountId, feeSettingData) => {
    if(feeSettingData.length < 1) throw new Error('fees length is 0')
    const COMMISSION_DB = buildAccountCommissionDBPath(accountId)
    if(!existsSync(COMMISSION_DB)) throw new Error('commission.db is not exist!')
    return new Promise((resolve, reject) => {
        const keys = Object.keys(feeSettingData[0], {})
        const q = [...keys].fill("?")
        runClearDB(COMMISSION_DB, 'commission')
        .then(() => {
            runBatchInsertDB(COMMISSION_DB, `INSERT INTO commission(${keys.join(", ")}) VALUES (${q.join(", ")})`, feeSettingData.map(f => [...Object.values(f)]))
            .then(() => resolve(true))
            .catch(err => reject(err))
        })
        .catch(err => reject(err))
    })
}

export const getFeeSettingData = (accountId) => {
    const COMMISSION_DB = buildAccountCommissionDBPath(accountId)
    const GLOBAL_COMMISSIO_DB = buildGloablCommissionDBPath()
    if(!existsSync(COMMISSION_DB)) {
        try{
            copySync(GLOBAL_COMMISSIO_DB, COMMISSION_DB)
        }catch(err){
            throw err
        }
    }
    return runSelectDB(COMMISSION_DB,`SELECT * FROM commission`)
}