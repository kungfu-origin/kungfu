import colors from 'colors';
import Table from '../public/Table';
import { calcuHeaderWidth, parseToString, parseAccountList, dealNum, dealStatus } from './utils';
import { sourceType } from '@/assets/config/accountConfig';
import { toDecimal } from '@/assets/js/utils';


function StrategyTable(){
    if (!(this instanceof StrategyTable)) {
        return new StrategyTable();
    }
    Table.call(this)
	this.headers = ['Strategy', 'Status', 'Pnl', 'PnlRt', 'dbPath'];
	this.columnWidth = [10, 8, 8, 6, 'auto']
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
StrategyTable.prototype.refresh = function(strategyData, processStatus){
	//accont (td + md)
	strategyData = Object.values(strategyData || {});
	const strategyListData = strategyData.map(s => {
		const strategyId = s.strategy_id;
        const strategyProcess = dealStatus(processStatus[strategyId]);
        const pathLen = s.strategy_path.length;
        const strategyPath = pathLen < 15 ? s.strategy_path :  `...${s.strategy_path.slice(pathLen - 15)}`
		return parseToString([
            strategyId,
            strategyProcess,
            '--',
            '--',
            strategyPath
        ].slice(0, this.headers.length), this.columnWidth)
    })
	this.table.setItems(strategyListData)
}

export default StrategyTable()