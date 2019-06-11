import Table from '../public/Table';
import { parseToString, dealNum, dealStatus, calcuHeaderWidth } from './utils';

function StrategyTable(){
    if (!(this instanceof StrategyTable)) {
        return new StrategyTable();
    }
    Table.call(this)
	this.headers = ['Strategy', 'Status', 'Pnl', 'PnlRt', 'dbPath'];
	this.columnWidth = [10, 8, 8, 8, 'auto']
}

StrategyTable.prototype = new Table();

StrategyTable.prototype.getData = function(globalData={}){
	return this.getDataMethod().then(list => {
        let strategyData = {};
        list.forEach(l => strategyData[l.strategy_id] = l)
        return strategyData
    })
}
/**
 * @param  {Object} strategyData
 * @param  {Object} processStatus
 */
StrategyTable.prototype.refresh = function(strategyData, processStatus, pnlData={}){
    const  t = this;
	strategyData = Object.values(strategyData || {});
	const strategyListData = strategyData.map(s => {
		const strategyId = s.strategy_id;
        const strategyProcess = dealStatus(processStatus[strategyId]);
        const pathLen = s.strategy_path.length;
        const strategyPath = pathLen < 15 ? s.strategy_path :  `...${s.strategy_path.slice(pathLen - 15)}`
        const pnlDataToStrat = pnlData[strategyId] || {};
        const accumPnl = dealNum(pnlDataToStrat.accumulated_pnl)
        const accumPnlRatio = dealNum(pnlDataToStrat.accumulated_pnl_ratio, true)
        
		const r = /m([^%]+)\u001b/
        const lw = accumPnlRatio.match(r) === null ? accumPnlRatio : accumPnlRatio.match(r)[1];
		const len = lw.length;
        return parseToString([
            strategyId,
            strategyProcess,
            accumPnl,
            accumPnlRatio,
            strategyPath
        ], calcuHeaderWidth(t.headers, t.columnWidth), t.pad, true)
    })
	t.table.setItems(strategyListData)
}

export default StrategyTable()