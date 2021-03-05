import Table from '@/assets/components/Table';
import { calcuHeaderWidth, parseToString } from "@/assets/scripts/utils";
import { aliveOrderStatusList } from '@kungfu-trader/kungfu-shared/config/tradingConfig';
const colors = require('colors/safe');

interface OrdersData {
	[propName: string]: OrderData
}

class OrderTable extends Table {
	type: string;
	headers: string[];
	ordersList: OrderData[];
	columnWidth: number[];
	constructor(type: string) {
		super();
		this.type = type;
		this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Filled/Not', 'Status', 
			type === 'account' ? 'Strate' : 'AccountId',
			'System La',
			'Network La'
		]    
		this.columnWidth = [10, 0, 0, 0, 8, 12, 8, 9, 10, 10];
		this.ordersList = []
	}

	setItems(orderDataList: OrderData[]) {
		this.refresh(orderDataList.slice(0, 500));
	}
	/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */
	refresh(ordersList: OrderData[]){
		const t = this;
		const orderListData = ordersList.map(o => {
			let side = o.side || '';
			if(side.toLowerCase() === 'buy') side = colors.red(side);
			else if(side.toLowerCase() === 'sell') side = colors.green(side);
			let offset = o.offset || ''
			if(offset.toLowerCase() === 'open') offset = colors.red(offset);
			else if(offset.toLowerCase() === 'close') offset = colors.green(offset);
			let statusName = o.statusName || ''
			if([3, 5, 6].includes(+o.status)) statusName = colors.green(statusName);
			else if(+o.status === 4) statusName = colors.red(statusName);
			else statusName = statusName;

			let last = o.clientId;
			if(t.type === 'strategy') {
				last = o.accountId
			};
			return parseToString(
				[
					o.updateTime,
					o.instrumentId,
					side,
					offset,
					o.limitPrice,
					o.volumeTraded,
					statusName,
					last,
					o.latencySystem,
					o.latencyNetwork
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