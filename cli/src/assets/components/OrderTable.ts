import Table from '@/assets/components/Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString } from "@/assets/scripts/utils";

const colors = require('colors/safe');

interface OrdersData {
	[propName: string]: OrderData
}

class OrderTable extends Table {
	type: string;
	headers: string[];
	ordersData: OrdersData;
	ordersList: OrderData[];
	constructor(type: string) {
		super();
		this.type = type;
		this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 
			type === 'account' ? 'Strate' : 'AccountId'
		]    
		this.columnWidth = [18, 0, 0, 0, 8, 12, 8, 9];
		this.ordersData = {};
		this.ordersList = []
	}

	setItems(orderData: OrderData) {
		this.ordersData[orderData.id] = orderData;
		this.ordersList = Object.values(this.ordersData || {}).sort((a: OrderData, b: OrderData) => {
			return  b.updateTimeNum - a.updateTimeNum
		})
		this.refresh();
	}
	/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */
	refresh(){
		const t = this;
		const orderListData = t.ordersList.map(o => {
			let side = o.side;
			if(side.toLowerCase() === 'buy') side = colors.red(side);
			else if(side.toLowerCase() === 'sell') side = colors.green(side);
			let offset = o.offset
			if(offset.toLowerCase() === 'open') offset = colors.red(offset);
			else if(offset.toLowerCase() === 'close') offset = colors.green(offset);
			let statusName = o.statusName
			if([3, 5, 6].indexOf(+o.status) !== -1) statusName = colors.green(statusName);
			else if(+o.status === 4) statusName = colors.red(statusName);
			else statusName = statusName;

			let last = o.clientId;
			if(t.type === 'strategy') {
				last = o.accountId
			};
			return parseToString(
				[
					o.updateTime.slice(2),
					o.instrumentId,
					side,
					offset,
					o.limitPrice,
					o.volumeTraded,
					statusName,
					last
				], 
				calcuHeaderWidth(t.headers, t.columnWidth), 
				t.pad
			)

		})
		t.table.setItems(orderListData)
		if(!t.table.childList.focused) {
			t.table.childList.select(0);
			t.table.childList.setScrollPerc(0);
		}
	}
}




export default (type: string) => new OrderTable(type)