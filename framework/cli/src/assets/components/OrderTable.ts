import { calcHeaderWidth, parseToString } from '../methods/utils';
import Table from './Table';
import colors from 'colors';
import {
  OffsetEnum,
  OrderStatusEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  dealOffset,
  dealOrderStatus,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  UnfinishedOrderStatus,
  WellFinishedOrderStatus,
} from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

class OrderTable extends Table {
  type: string;

  constructor(type: string) {
    super();
    this.type = type;
    this.headers = [
      'UpdateTime',
      'Ticker',
      'Side',
      'Offset',
      'Price',
      'Filled/Not',
      'Status',
      type === 'account' ? 'Strate' : 'AccountId',
      'System LA',
      'Network LA',
    ];
    this.columnWidth = [10, 0, 0, 0, 8, 12, 8, 9, 10, 10];
  }

  setItems(orderDataList: KungfuApi.OrderResolved[]) {
    this.refresh(orderDataList.slice(0, 50));
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

  dealOrderStatus(orderStatus: OrderStatusEnum) {
    const name = dealOrderStatus(orderStatus).name;
    if (WellFinishedOrderStatus.includes(orderStatus)) {
      return colors.green(name);
    } else if (orderStatus === OrderStatusEnum.Error) {
      return colors.red(name);
    } else {
      return name;
    }
  }

  dealLast(order: KungfuApi.OrderResolved) {
    if (this.type === 'strategy') {
      return order.source_uname;
    } else {
      return order.dest_uname;
    }
  }

  refresh(ordersList: KungfuApi.OrderResolved[]) {
    const orderListData = ordersList.map((o) => {
      const side = this.dealSide(o.side);
      const offset = this.dealOffset(o.offset);
      const orderStatus = this.dealOrderStatus(o.status);
      const last = this.dealLast(o);

      return parseToString(
        [
          o.update_time_resolved,
          o.instrument_id,
          side,
          offset,
          o.limit_price,
          Number(o.volume_traded),
          orderStatus,
          last,
          o.latency_system,
          o.latency_network,
        ],
        calcHeaderWidth(this.headers, this.columnWidth),
        this.pad,
      );
    });

    this.table.setItems(orderListData);
    if (!this.table.childList.focused) {
      this.table.childList.select(0);
      this.table.childList.setScrollPerc(0);
    }
  }
}

export default (type: string) => new OrderTable(type);
