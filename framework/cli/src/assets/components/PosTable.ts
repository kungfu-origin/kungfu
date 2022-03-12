import { DirectionEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { dealDirection } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import colors from 'colors';
import { calcHeaderWidth, colorNum, parseToString } from '../methods/utils';
import Table from './Table';

export class PosTable extends Table {
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

  setItems(positions: KungfuApi.PositionResolved[]) {
    this.refresh(positions);
  }

  dealDirection(direction: DirectionEnum) {
    const name = dealDirection(direction).name;
    if (direction === DirectionEnum.Long) {
      return colors.red(name);
    } else {
      return colors.green(name);
    }
  }

  refresh(positions: KungfuApi.PositionResolved[]) {
    const posListData = positions.map((p: KungfuApi.PositionResolved) => {
      const direction = this.dealDirection(p.direction);

      return parseToString(
        [
          p.instrument_id_resolved,
          direction,
          Number(p.yesterday_volume),
          Number(p.volume - p.yesterday_volume),
          Number(p.volume),
          p.avg_open_price,
          p.last_price,
          colorNum(p.unrealized_pnl),
        ],
        calcHeaderWidth(this.headers, this.columnWidth),
        this.pad,
      );
    });
    this.table.setItems(posListData);
    if (!this.table.childList.focused) this.table.childList.setScrollPerc(0);
  }
}

export default () => new PosTable();
