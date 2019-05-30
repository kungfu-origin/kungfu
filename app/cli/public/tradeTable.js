import Table from '../public/Table';
import { calcuHeaderWidth, parseToString, posDirection } from './utils';


function OrderTable(){
    if (!(this instanceof OrderTable)) {
        return new OrderTable();
    }
    Table.call(this);
	this.headers = ['OrderTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 'Strat']    
}

OrderTable.prototype = new Table();

OrderTable.prototype.getData = function(accountId){
	return this.getDataMethod(accountId, {}).then(pos => {
        return pos
	})
}
/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
OrderTable.prototype.refresh = function(orderData){

	const orderListData = orderData.map(o => {
		let side = sideName[o.side] ? sideName[o.side] : '--';
		// if(side === 'buy') side = colors.red(side);
		// else if(side === 'sell') side = colors.green(side);

		let offset = offsetName[o.offset] ? offsetName[o.offset] : '--';
		// if(offset === 'open') offset = colors.red(offset);
		// else if(offset.indexOf('close') !== -1) offset = colors.green(offset);

		let status = orderStatus[o.status]
		// if([3, 5, 6].indexOf(o.status) !== -1) status = colors.green(status);
		// else if(+o.status === 4) status = colors.red(status);
		// else status = colors.grey(status);

		return parseToString([
			o.insert_time && moment(o.insert_time/1000000).format("HH:mm:ss"),
			o.instrument_id,
			side,
			offset,
			o.limit_price,
			o.volume_traded + '/' + o.volume_left,
			status,
			o.client_id
		], calcuHeaderWidth(this.headers))
	})
	this.table.setItems(orderListData)
}

export default OrderTable()