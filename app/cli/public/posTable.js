import Table from '../public/Table';
import { calcuHeaderWidth, parseToString, posDirection } from './utils';
import { toDecimal } from '@/assets/js/utils';

function PosTable(){
    if (!(this instanceof PosTable)) {
        return new PosTable();
    }
    
    Table.call(this)
	this.headers = ['Ticker', 'Dir', 'Yest', 'Total', ' UnRealPnl']
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
		// if(direction === 'L') direction = colors.red(direction);
		// else direction = colors.green(direction);

		let unRealizedPnl = toDecimal(p.unrealized_pnl);
		return parseToString([
			p.instrument_id,
			p.direction,
			p.yesterday_volume,
			p.volume,
			unRealizedPnl
		], calcuHeaderWidth(this.headers, this.columnWidth))
    })
	this.table.setItems(posListData)
}

export default PosTable()