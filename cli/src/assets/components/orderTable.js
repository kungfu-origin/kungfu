import colors from 'colors';
import moment from 'moment';
import Table from './Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString, buildDateRange } from "../utils";

function OrderTable(){
    if (!(this instanceof OrderTable)) {
        return new OrderTable();
    }
    Table.call(this);
	this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 'Strate']    
}

OrderTable.prototype = new Table();

OrderTable.prototype.getData = function(currentId){
	if(!currentId) return new Promise(resolve => resolve([]))
	return this.getDataMethod(currentId, {dateRange: buildDateRange()}).then(orders => {
        return orders
	})
}
/**
 * @param  {Object} accountData
 * @param  {Object} processStatus
 */
OrderTable.prototype.refresh = function(orderData, type){
	const orderListData = orderData.map(o => {
		let side = sideName[o.side] ? sideName[o.side] : '--';
		if(side === 'buy') side = colors.red(side);
		else if(side === 'sell') side = colors.green(side);
		let offset = offsetName[o.offset] ? offsetName[o.offset] : '--';
		if(offset === 'open') offset = colors.red(offset);
		else if(offset.indexOf('close') !== -1) offset = colors.green(offset);
		let status = orderStatus[o.status]
		if([3, 5, 6].indexOf(+o.status) !== -1) status = colors.green(status);
		else if(+o.status === 4) status = colors.red(status);
		else status = colors.grey(status);

		let last = o.client_id;
		if(type === 'strategy') last = o.account_id;
		return parseToString([
			o.insert_time && moment(o.insert_time/1000000).format("HH:mm:ss"),
			o.instrument_id,
			side,
			offset,
			o.limit_price,
			o.volume_traded + '/' + o.volume_left,
			status,
			last
		], calcuHeaderWidth(this.headers, this.columnWidth), this.pad)
	})
	this.table.setItems(orderListData)
	if(!this.table.childList.focused) this.table.childList.setScrollPerc(0)
}

export default OrderTable()