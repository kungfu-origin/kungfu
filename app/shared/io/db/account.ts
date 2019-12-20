import { buildDateRange } from '__gUtils/busiUtils';
import { runSelectDB, runInsertUpdateDeleteDB } from '__gUtils/dbUtils';
import { ACCOUNTS_DB, LIVE_TRADING_DATA_DB } from '__gConfig/pathConfig';
import moment from "moment"


export const getTdList = () => {
    return runSelectDB(ACCOUNTS_DB, 'SELECT * FROM td_config ORDER BY account_id')
}

export const getMdList = () => {
    return runSelectDB(ACCOUNTS_DB, 'SELECT * FROM md_config ORDER BY source_name')
}

export const addTd = (accountId: string, sourceName: string, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'INSERT INTO td_config(account_id, source_name, config) VALUES (?, ?, ?)', [accountId, sourceName, config])
}

export const addMd = (sourceName: string, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'INSERT INTO md_config(source_name, config) VALUES (?, ?)', [sourceName, config])
}

export const updateTdConfig = (accountId: string, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE td_config SET config = ? WHERE account_id = ?', [config, accountId])
}

export const updateMdConfig = (sourceName: string, config: any) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, 'UPDATE md_config SET config = ? WHERE source_name = ?', [config, sourceName])
}

export const deleteTd = (accountId: string) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, "DELETE FROM td_config WHERE account_id = ?", accountId)
}

export const deleteMd = (sourceName: string) => {
    return runInsertUpdateDeleteDB(ACCOUNTS_DB, "DELETE FROM md_config WHERE source_name = ?", sourceName)
}

export const getExistedMdSources = () => getMdList()
    .then((mdList: Md[]) => {
        return mdList.map(m => m.source_name)
    })

/**
 * 获取账户资金情况
 * 
 */
export const getAccountAsset = () => {
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        'SELECT * FROM asset' + 
        ' WHERE ledger_category = 0'
    )
}

export const getAccountAssetById = (accountId: string) => {
    accountId = accountId.toAccountId();
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        'SELECT * FROM asset' + 
        ' WHERE ledger_category = 0' +
        ` AND account_id = "${accountId}"`
    )
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
        `SELECT * FROM position` + 
        ` WHERE ledger_category = 0` + 
        ` AND volume != 0` +
        ` AND account_id = "${accountId}"` + 
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
export const getAccountOrder = (accountId: string, { id, dateRange }: TradingDataFilter, tradingDay?: string) => {
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
        ` AND trading_day >= ${filterDate[0]}` +
        ` AND trading_day <= ${filterDate[1]}` +
        (dateRange.length ? `` : ` AND status NOT IN (0,3,4,5,6)`) + //有日期筛选的时候,获取所有状态的数据；无日期的时候，获取的是当天的且未完成的
        ` ORDER BY insert_time DESC`
    )
}

/**
 * 获取账户成交情况
 * 
 */
export const getAccountTrade = (accountId: string, { id, dateRange }: TradingDataFilter, tradingDay?: string) => {
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
        ` AND trading_day >= ${filterDate[0]}` +
        ` AND trading_day <= ${filterDate[1]}` +
        ` ORDER BY trade_time DESC`
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
    tradingDay = moment(tradingDay).format('YYYYMMDD');
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM asset_snapshot` + 
        ` WHERE ledger_category = 0` +
        ` AND trading_day = '${tradingDay}'` +
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
        'SELECT * FROM (select * from asset_snapshot WHERE ledger_category = 0 ORDER BY update_time DESC)' + 
        ` WHERE account_id='${accountId}'` +
        ` GROUP BY trading_day`
    )
}



