import colors from 'colors';
import Table from '../public/Table';
import { calcuHeaderWidth, parseToString, posDirection } from './utils';
import { toDecimal } from '@/assets/js/utils';

function PosTable(){
    if (!(this instanceof PosTable)) {
        return new PosTable();
    }
    
    Table.call(this)
	this.headers = ['Ticker', 'Dir', 'Yest', 'Total', 'AverCost', 'LastPrice', 'UnRealPnl']
	this.columnWidth = [0, 5]
}

PosTable.prototype = new Table();
PosTable.prototype.getData = function(currentId){
	if(!currentId) return new Promise(resolve => resolve({}))
	return this.getDataMethod(currentId, {}).then(pos => {
        return pos
	})
}
/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
PosTable.prototype.refresh = function(posData){
	posData = Object.values(posData || {})
	const posListData = posData.map(p => {
		let direction = posDirection[p.direction]
		if(direction === 'Long') direction = colors.red(direction);
		else direction = colors.green(direction);
		let unRealizedPnl = toDecimal(p.unrealized_pnl);
		if(unRealizedPnl > 0) unRealizedPnl = colors.red(unRealizedPnl)
		else if(unRealizedPnl < 0) unRealizedPnl = colors.green(unRealizedPnl)

		return parseToString([
			p.instrument_id,
			direction,
			p.yesterday_volume,
			p.volume,
			toDecimal(p.cost_price) || '--',
			toDecimal(p.last_price) || '--',
			unRealizedPnl
		], calcuHeaderWidth(this.headers, this.columnWidth), this.pad)
    })
	this.table.setItems(posListData)
	if(!this.table.childList.focused) this.table.childList.setScrollPerc(0)
}

export default PosTable()