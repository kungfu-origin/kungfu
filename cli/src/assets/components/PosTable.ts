import colors from 'colors';
import Table from './Table';
import { calcuHeaderWidth, parseToString, posDirection, dealNum } from '@/assets/scripts/utils';
import { toDecimal } from '__gUtils/busiUtils';

interface PosObject {
	[propName: string]: PosData
}

class PosTable extends Table {
	headers: string[];
	columnWidth: Array<number>;
	posData: PosObject;
	constructor(){
        super();
		this.headers = ['Ticker', 'Dir', 'Yest', 'Today/Avail', 'Total', 'Open', 'Last', 'UnrealPnl']
		this.columnWidth = [0, 4];
		this.posData = {};
	}


	/**
	 * @param  {Object} accountData
	 * @param  {Object} processStatus
	 */

	setItems(posDataList: PosData[]) {
		const t = this;
		posDataList.forEach((p: PosData) => {
			t.posData[p.id] = p;
		})
		t.refresh(t.posData)
	}

	refresh(posData: PosObject) {
		const t = this;
		const posListData = Object.values(posData || {}).map((p: PosData) => {
			let direction = posDirection[p.direction]
			if(direction === 'Long') direction = colors.red(direction);
			else direction = colors.green(direction);
			let unRealizedPnl = toDecimal(+p.unRealizedPnl);
			if(+unRealizedPnl > 0) unRealizedPnl = colors.red(unRealizedPnl)
			else if(+unRealizedPnl < 0) unRealizedPnl = colors.green(unRealizedPnl)
			
			return parseToString(
				[
					p.instrumentId,
					direction,
					p.yesterdayVolume,
					p.todayVolume,
					p.totalVolume,
					dealNum(+p.avgPrice),
					dealNum(+p.lastPrice),
					unRealizedPnl
				], 
				calcuHeaderWidth(t.headers, t.columnWidth), 
				t.pad
			)
		})
		t.table.setItems(posListData)
		if(!t.table.childList.focused) t.table.childList.setScrollPerc(0)
	}
	
}

export default () => new PosTable()