import colors from 'colors';
import Table from './Table';
import {
  calcHeaderWidth,
  parseToString,
  dealNum,
} from '@/assets/scripts/utils';
import { toDecimal } from '__gUtils/busiUtils';

interface StringToPosData {
  [propName: string]: PosData;
}

class PosTable extends Table {
  headers: string[];
  columnWidth: Array<number>;
  constructor() {
    super();
    this.headers = [
      'Ticker',
      'Dir',
      'Yesterday',
      'Today',
      'Total',
      'Open',
      'Last',
      'UnrealPnl',
    ];
    this.columnWidth = [8, 4, 9, 9, 9, 9, 9, 12];
  }

  /**
   * @param  {Object} accountData
   * @param  {Object} processStatus
   */

  setItems(posData: StringToPosData) {
    this.refresh(posData);
  }

  refresh(posData: StringToPosData) {
    const t = this;
    const posListData = Object.values(posData || {}).map((p: PosData) => {
      let direction = p.direction;
      if (direction === 'Long') direction = colors.red(direction);
      else direction = colors.green(direction);
      let unRealizedPnl = toDecimal(+p.unRealizedPnl);
      if (+unRealizedPnl > 0) unRealizedPnl = colors.red(unRealizedPnl);
      else if (+unRealizedPnl < 0) unRealizedPnl = colors.green(unRealizedPnl);

      return parseToString(
        [
          p.instrumentId,
          direction,
          dealNum(p.yesterdayVolume),
          dealNum(p.todayVolume),
          dealNum(p.totalVolume),
          dealNum(+p.avgPrice),
          dealNum(+p.lastPrice),
          unRealizedPnl,
        ],
        calcHeaderWidth(t.headers, t.columnWidth),
        t.pad,
      );
    });
    t.table.setItems(posListData);
    if (!t.table.childList.focused) t.table.childList.setScrollPerc(0);
  }
}

export default () => new PosTable();
