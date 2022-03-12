import { calcHeaderWidth, parseToString } from '../methods/utils';
import Table from './Table';
import colors from 'colors';
import {
  KfCategoryTypes,
  OffsetEnum,
  OrderStatusEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  dealOffset,
  dealOrderStatus,
  dealSide,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { WellFinishedOrderStatus } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';

export class OrderTable extends Table {
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
      'Filled/Not',
      'Status',
      category === 'td' ? 'StratId' : 'AccountId',
      'System LA',
      'Network LA',
    ];
    this.columnWidth = [10, 0, 0, 0, 8, 12, 8, 9, 10, 10];
  }

  setItems(orderDataList: KungfuApi.OrderResolved[]) {
    this.refresh(orderDataList.slice(0, 10));
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
    if (this.category === 'strategy') {
      return order.source_uname;
    } else {
      return order.dest_uname;
    }
  }

  refresh(ordersList: KungfuApi.OrderResolved[]) {
    const orderListData = ordersList.map((order) => {
      const side = this.dealSide(order.side);
      const offset = this.dealOffset(order.offset);
      const orderStatus = this.dealOrderStatus(order.status);
      const last = this.dealLast(order);

      return parseToString(
        [
          order.update_time_resolved,
          order.instrument_id,
          side,
          offset,
          order.limit_price,
          `${order.volume_traded}/${order.volume}`,
          orderStatus,
          last,
          order.latency_system,
          order.latency_network,
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

export default (category: KfCategoryTypes) => new OrderTable(category);
