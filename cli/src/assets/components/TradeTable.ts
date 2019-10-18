import Table from '@/assets/components/Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString } from "@/assets/scripts/utils";
const colors = require('colors');


class TradeTable extends Table {
	type: string;
	headers: string[];
	columnWidth: number[];
	tradeList: TradeData[];

	constructor(type: string) {
		super()
		this.type = type;
		this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Volume', 
			type === 'account' ? 'Strate' : 'AccountId '
		]
		this.columnWidth = [18, 0, 0, 0, 8, 6, 9];
		this.tradeList = [];
	}

	setItems(tradeDataList: TradeData[]) {
		tradeDataList.forEach((tradeData: TradeData) => {
			this.tradeList.unshift(tradeData)			
		})
		this.tradeList = this.tradeList.slice(0, 500)
		this.refresh()
	}
		/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */
	refresh(){
		const t = this;
		const tradeListData = t.tradeList.map((trade: TradeData) => {
			let side = trade.side;
			if(side.toLowerCase() === 'buy') side = colors.red(side);
			else if(side.toLowerCase() === 'sell') side = colors.green(side);
			let offset = trade.offset
			if(offset.toLowerCase() === 'open') offset = colors.red(offset);
			else if(offset.toLowerCase() === 'close') offset = colors.green(offset);
			let last = trade.clientId;
			if(t.type === 'strategy') last = trade.accountId;
			return parseToString(
					[
					trade.updateTime.slice(2),
					trade.instrumentId,
					side,
					offset,
					trade.price,
					trade.volume,
					last
				], 
				calcuHeaderWidth(this.headers, this.columnWidth), 
				t.pad
			)
		})
		t.table.setItems(tradeListData);
		if(!t.table.childList.focused) {
			t.table.childList.select(0);
			t.table.childList.setScrollPerc(0);
		}
		t.table.render();
	}

}

export default (type: string) => new TradeTable(type)