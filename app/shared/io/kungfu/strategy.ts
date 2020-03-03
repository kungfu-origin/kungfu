import { getAllKfConfig, setKfConfig, removeKfConfig } from '__gUtils/kungfuUtils';

// /**
//  * 获取策略列表
//  */
// export const getStrategyList = () => {
//     return runSelectDB(STRATEGYS_DB, 'SELECT * FROM strategys')
// }

// /**
//  * 获取策略by id
//  * @param  {string} id
//  */
// export const getStrategyById = (id: string) => {
//     return runSelectDB(STRATEGYS_DB, "SELECT * FROM strategys WHERE strategy_id = ?", id)
// }

// /**
//  * 新建策略
//  * @param {String} id 交易任务名 
//  */

// export const addStrategy = (strategy_id: string, strategy_path: string) => {
//     const addTime = +new Date().getTime() * Math.pow(10,6)
//     return runInsertUpdateDeleteDB(STRATEGYS_DB, 'INSERT INTO strategys(strategy_id, strategy_path, add_time) VALUES (?, ?, ?)', [strategy_id, strategy_path || null, addTime])
// }

// /**
//  * 删除策略
//  * @param  {} strategy_id
//  */
// export const deleteStrategy = (strategy_id: string) => {
//     return runInsertUpdateDeleteDB(STRATEGYS_DB, "DELETE FROM strategys WHERE strategy_id = ?", strategy_id)
// }
 
// /**
//  * 更改策略路径
//  * @param  {String} strategy_id
//  * @param  {String} strategy_path
//  */
// export const updateStrategyPath = (strategy_id: string, strategy_path: string) => {
//     return runInsertUpdateDeleteDB(STRATEGYS_DB, 'UPDATE strategys SET strategy_path = ? WHERE strategy_id = ?', [strategy_path, strategy_id])    
// }


// export const getStrategyAssetById = (strategyId: string) => {
//     return runSelectDB(
//         LIVE_TRADING_DATA_DB, 
//         'SELECT * FROM asset' + 
//         ' WHERE ledger_category = 1' +
//         ` AND client_id = "${strategyId}"`
//     )
// }

// const getStrategyAddTime = async (strategyId: string): Promise<number | undefined> => {
//     //新建与之前重名策略，防止get之前的数据
//     const strategies = await getStrategyById(strategyId)
//     if(!strategies[0]) throw new Error('找不到该策略！');
//     return strategies[0].add_time;
// }
