import Table from '@/assets/components/Table';
import { offsetName, orderStatus, sideName, calcuHeaderWidth, parseToString } from "@/assets/scripts/utils";
const colors = require('colors');


class TradeTable extends Table {
	type: string;
	headers: string[];
	columnWidth: number[];

	constructor(type: string) {
		super()
		this.type = type;
		this.headers = ['UpdateTime', 'Ticker', 'Side', 'Offset', 'Price', 'Volume', 'Strate']
		this.columnWidth = [0, 0, 0, 0, 0, 6]
	}

		/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */
	refresh(tradesData: TradeData[]){
		const t = this;
		const tradeListData = tradesData.map((trade: TradeData) => {
			let side = sideName[trade.side] ? sideName[trade.side] : '--';
			if(side === 'buy') side = colors.red(side);
			else if(side === 'sell') side = colors.green(side);
			let offset = offsetName[trade.offset] ? offsetName[trade.offset] : '--';
			if(offset === 'open') offset = colors.red(offset);
			else if(offset.indexOf('close') !== -1) offset = colors.green(offset);
			let last = trade.clientId;
			if(t.type === 'strategy') last = trade.accountId;
			return parseToString(
					[
					trade.updateTime,
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
		if(!t.table.childList.focused) t.table.childList.setScrollPerc(0);
	}

}

export default (type: string) => new TradeTable(type)