import Table from '@/assets/components/Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString, buildDateRange } from "@/assets/scripts/utils";

const colors = require('colors');

interface OrderObject {
	[propName: string]: OrderData
}

class OrderTable extends Table {
	type: string;
	headers: string[];
	constructor(type: string) {
		super();
		this.type = type;
		this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 'Strate']    
	}

	/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */
	refresh(orderData: OrderObject){
		const t = this;
		const orderListData = Object.values(orderData || {}).map(o => {
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

			let last = o.clientId;
			if(t.type === 'strategy') last = o.accountId;
			return parseToString(
				[
					o.updateTime,
					o.instrumentId,
					side,
					offset,
					o.limitPrice,
					o.volumeTraded,
					status,
					last
				], 
				calcuHeaderWidth(t.headers, t.columnWidth), 
				t.pad
			)

		})
		t.table.setItems(orderListData)
		if(!t.table.childList.focused) t.table.childList.setScrollPerc(0)
	}
}




export default (type: string) => new OrderTable(type)