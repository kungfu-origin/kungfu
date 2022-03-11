import Table from '@/assets/components/Table';
import { calcHeaderWidth, parseToString } from '@/assets/scripts/utils';
const colors = require('colors');

class TradeTable extends Table {
  type: string;
  headers: string[];
  columnWidth: number[];

  constructor(type: string) {
    super();
    this.type = type;
    this.headers = [
      'UpdateTime',
      'Ticker',
      'Side',
      'Offset',
      'Price',
      'Volume',
      type === 'account' ? 'Strate' : 'AccountId ',
      'Trade LA',
    ];
    this.columnWidth = [10, 0, 0, 0, 8, 6, 9, 10];
  }

  setItems(tradeDataList: TradeData[]) {
    this.refresh(tradeDataList.slice(0, 500));
  }
  /**
   * @param  {Object} accountData
   * @param  {Object} processStatus
   */
  refresh(tradeList: TradeData[]) {
    const t = this;
    const tradeListData = tradeList.map((trade: TradeData) => {
      let side = trade.side || '';
      if (side.toLowerCase() === 'buy') side = colors.red(side);
      else if (side.toLowerCase() === 'sell') side = colors.green(side);
      let offset = trade.offset || '';
      if (offset.toLowerCase() === 'open') offset = colors.red(offset);
      else if (offset.toLowerCase() === 'close') offset = colors.green(offset);
      let last = trade.clientId;
      if (t.type === 'strategy') last = trade.accountId;
      return parseToString(
        [
          trade.updateTime,
          trade.instrumentId,
          side,
          offset,
          trade.price,
          trade.volume,
          last,
          trade.latencyTrade,
        ],
        calcHeaderWidth(this.headers, this.columnWidth),
        t.pad,
      );
    });
    t.table.setItems(tradeListData);
    if (!t.table.childList.focused) {
      t.table.childList.select(0);
      t.table.childList.setScrollPerc(0);
    }
  }
}

export default (type: string) => new TradeTable(type);
