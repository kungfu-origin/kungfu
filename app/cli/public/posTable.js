import Table from '../public/Table';
import { calcuHeaderWidth, parseToString, posDirection } from './utils';


function PosTable(){
    if (!(this instanceof PosTable)) {
        return new PosTable();
    }
    
    Table.call(this)
	this.headers = ['Ticker', 'Dir', 'Yest', 'Total', ' UnRealPnl']
}

PosTable.prototype = new Table();

PosTable.prototype.getData = function(accountId){
	return this.getDataMethod(accountId, {}).then(pos => {
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

		let unRealizedPnl = new Number(p.unrealized_pnl).toFixed(2);
		return parseToString([
			p.instrument_id,
			p.direction,
			p.yesterday_volume,
			p.volume,
			unRealizedPnl
		], calcuHeaderWidth(this.headers))
	})
	this.table.setItems(posListData)
}

export default PosTable()