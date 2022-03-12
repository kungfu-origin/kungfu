import {
  KfCategoryTypes,
  OffsetEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  dealOffset,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import colors from 'colors';
import { calcHeaderWidth, parseToString } from '../methods/utils';
import Table from './Table';

export class TradeTable extends Table {
  category: KfCategoryTypes;

  constructor(category: KfCategoryTypes) {
    super();
    this.category = category;
    this.headers = [
      'UpdateTime',
      'Ticker',
      'Side',
      'Offset',
      'Price',
      'Volume',
      category === 'td' ? 'StratId' : 'AccountId ',
      'Trade LA',
    ];
    this.columnWidth = [10, 0, 0, 0, 8, 6, 9, 10];
  }

  setItems(tradeDataList: KungfuApi.TradeResolved[]) {
    this.refresh(tradeDataList.slice(0, 10));
  }

  dealSide(side: SideEnum) {
    const name = dealSide(side).name;
    if (side === SideEnum.Buy) {
      return colors.red(name);
    }

    if (side === SideEnum.Sell) {
      return colors.green(name);
    }
  }

  dealOffset(offset: OffsetEnum) {
    const name = dealOffset(offset).name;
    if (offset === OffsetEnum.Open) {
      return colors.red(name);
    } else {
      return colors.green(name);
    }
  }

  dealLast(trade: KungfuApi.TradeResolved) {
    if (this.category === 'strategy') {
      return trade.source_uname;
    } else {
      return trade.dest_uname;
    }
  }

  refresh(tradeList: KungfuApi.TradeResolved[]) {
    const tradeListData = tradeList.map((trade: KungfuApi.TradeResolved) => {
      const side = this.dealSide(trade.side);
      const offset = this.dealOffset(trade.offset);
      const last = this.dealLast(trade);

      return parseToString(
        [
          trade.trade_time_resolved,
          trade.instrument_id,
          side,
          offset,
          trade.price,
          Number(trade.volume),
          last,
          trade.latency_trade,
        ],
        calcHeaderWidth(this.headers, this.columnWidth),
        this.pad,
      );
    });

    this.table.setItems(tradeListData);
    if (!this.table.childList.focused) {
      this.table.childList.select(0);
      this.table.childList.setScrollPerc(0);
    }
  }
}

export default (type: string) => new TradeTable(type);
