import { runSelectDB, runInsertUpdateDeleteDB } from '__gUtils/dbUtils';
import { buildDateRange } from '__gUtils/busiUtils';
import {
    STRATEGYS_DB, 
    LIVE_TRADING_DATA_DB
} from '__gConfig/pathConfig';
import moment from "moment"

/**
 * 获取策略列表
 */
export const getStrategyList = () => {
    return runSelectDB(STRATEGYS_DB, 'SELECT * FROM strategys')
}

/**
 * 获取策略by id
 * @param  {string} id
 */
export const getStrategyById = (id: string) => {
    return runSelectDB(STRATEGYS_DB, "SELECT * FROM strategys WHERE strategy_id = ?", id)
}

/**
 * 新建策略
 * @param {String} id 交易任务名 
 */

export const addStrategy = (strategy_id: string, strategy_path: string) => {
    const addTime = +new Date().getTime() * Math.pow(10,6)
    return runInsertUpdateDeleteDB(STRATEGYS_DB, 'INSERT INTO strategys(strategy_id, strategy_path, add_time) VALUES (?, ?, ?)', [strategy_id, strategy_path || null, addTime])
}

/**
 * 删除策略
 * @param  {} strategy_id
 */
export const deleteStrategy = (strategy_id: string) => {
    return runInsertUpdateDeleteDB(STRATEGYS_DB, "DELETE FROM strategys WHERE strategy_id = ?", strategy_id)
}
 
/**
 * 更改策略路径
 * @param  {String} strategy_id
 * @param  {String} strategy_path
 */
export const updateStrategyPath = (strategy_id: string, strategy_path: string) => {
    return runInsertUpdateDeleteDB(STRATEGYS_DB, 'UPDATE strategys SET strategy_path = ? WHERE strategy_id = ?', [strategy_path, strategy_id])    
}

/**
 * 获取某策略下委托
 */
export const getStrategyOrder = async(strategyId: string, {id, dateRange}: TradingDataFilter, tradingDay: string) => {
    dateRange = dateRange || [];
    id = id || ''
    //新建与之前重名策略，防止get之前的数据
    const strategys = await getStrategyById(strategyId)
    if(!strategys[0]) throw new Error('找不到该策略！');
    const strategyAddTime = strategys[0].add_time;
    const filterDate = buildDateRange(dateRange, tradingDay, strategyAddTime)    
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM orders` +
        ` WHERE client_id = '${strategyId}'` + 
        ` AND (order_id LIKE '%${id}%'` + 
        ` OR instrument_id LIKE '%${id}%')` + //有id筛选的时候
        ` AND insert_time >= ${filterDate[0]}` + 
        ` AND insert_time < ${filterDate[1]}` +
        (dateRange.length ? `` : ` AND status NOT IN (3,4,5,6)`) + //有日期筛选的时候,获取所有状态的数据；无日期的时候，获取的是当天的且未完成的
        ` ORDER BY insert_time`
    )
}

/**
 * 获取某策略下成交
 */
export const getStrategyTrade = async(strategyId: string, { id, dateRange }: TradeInputData, tradingDay: string) => {
    dateRange = dateRange || [];
    id = id || ''
    //新建与之前重名策略，防止get之前的数据    
    const strategys = await getStrategyById(strategyId)
    if(!strategys[0]) throw new Error('找不到该策咯！');
    const strategyAddTime = strategys[0].add_time;
    const filterDate = buildDateRange(dateRange, tradingDay, strategyAddTime)
    return runSelectDB(
        LIVE_TRADING_DATA_DB,
        `SELECT rowId, * FROM trades` +
        ` WHERE client_id = '${strategyId}'` + 
        ` AND instrument_id LIKE '%${id}%'` + //有id筛选的时候
        ` AND trade_time >= ${filterDate[0]}` + 
        ` AND trade_time < ${filterDate[1]}` +
        ` ORDER BY trade_time`
    )
}

/**
 * 获取某策略下的持仓
 */
export const getStrategyPos = (strategyId: string, { instrumentId }: TradingDataFilter) => {
    instrumentId = instrumentId || '';
    return runSelectDB(
        LIVE_TRADING_DATA_DB,
        `SELECT * FROM portfolio_position` + 
        ` WHERE client_id = "${strategyId}"` + 
        ` AND instrument_id LIKE '%${instrumentId}%'` +
        ` ORDER BY instrument_id`
    )
    .then((pos: PosInputData[]): any => {
        let posList: any = {}
        //策略的pos是一条一条成交记录？
        pos.map((item: PosInputData): void => {
            let key = item.instrument_id + item.direction
            if(posList[key]) {
                posList[key].yesterday_volume = posList[key].yesterday_volume + item.yesterday_volume
                posList[key].volume = posList[key].volume + item.volume
                posList[key].last_price = posList[key].last_price + item.last_price
                posList[key].margin = posList[key].margin + item.margin
            }else{
                posList[key] = item
            }
        })
        return Object.values(posList);
    })
}

/**
 * 获取某策略下收益曲线分钟线
 */
export const getStrategyPnlMin = (strategyId: string, tradingDay: string) => {
    if(!tradingDay) throw new Error('无交易日！')
    return runSelectDB(
        LIVE_TRADING_DATA_DB, 
        `SELECT * FROM portfolio_snapshot` + 
        ` WHERE trading_day = '${tradingDay}'` + 
        ` AND client_id = '${strategyId}'`
    )
}

/**
 * 获取某策略下收益曲线日线
 */
export const getStrategyPnlDay = (strategyId: string) => {
    return runSelectDB(
        LIVE_TRADING_DATA_DB,
        'SELECT * FROM (select * from portfolio_snapshot ORDER BY update_time DESC)' + 
        ` where client_id = '${strategyId}'` +
        ` GROUP BY trading_day`
    )
}


interface LastPnl {
    lastPnl: number;
    strategyId: string;
}

export const getStrategysPnl = (ids: string[], tradingDay: string): Promise<LastPnl[]> => {
    if(!tradingDay) throw new Error('无交易日！')
    const promises = ids.map(id => getStrategyPnlMin(id, tradingDay).then((res: any) => {
        const resLen = Object.keys(res || {}).length;
        let lastIndex = 0;
        if(resLen > 0) lastIndex = resLen - 1;
        return { lastPnl: res[lastIndex], strategyId: id }
    }))
    return Promise.all(promises)
}



