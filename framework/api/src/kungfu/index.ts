import dayjs from 'dayjs';
import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
  dealDirection,
  dealHedgeFlag,
  dealInstrumentType,
  dealIsSwap,
  dealLocationUID,
  dealOffset,
  dealOrderStat,
  dealOrderStatus,
  dealPriceType,
  dealSide,
  dealTimeCondition,
  dealVolumeCondition,
  getIdByKfLocation,
  getMdTdKfLocationByProcessId,
  isShotable,
  isT0,
  kfLogger,
  resolveAccountId,
  resolveClientId,
} from '../utils/busiUtils';
import {
  HistoryDateEnum,
  LedgerCategoryEnum,
  InstrumentTypeEnum,
  BasketVolumeTypeEnum,
} from '../typings/enums';
import { ExchangeIds } from '../config/tradingConfig';

export const kf = kungfu();

kfLogger.info('Load kungfu node');

export const configStore = kf.ConfigStore(KF_RUNTIME_DIR);
export const riskSettingStore = kf.RiskSettingStore(KF_RUNTIME_DIR);
export const history = kf.History(KF_RUNTIME_DIR);
export const commissionStore = kf.CommissionStore(KF_RUNTIME_DIR);
export const basketStore = kf.BasketStore(KF_RUNTIME_DIR);
export const basketInstrumentStore = kf.BasketInstrumentStore(KF_RUNTIME_DIR);
export const longfist = kf.longfist;

export const dealKfTime = (nano: bigint, date = false): string => {
  if (nano === BigInt(0)) {
    return '--';
  }

  if (date) {
    return kf.formatTime(nano, '%m/%d %H:%M:%S.%N').slice(0, 18);
  }
  return kf.formatTime(nano, '%H:%M:%S.%N').slice(0, 12);
};

export const dealTradingDataItem = (
  item: KungfuApi.TradingDataTypes,
  watcher: KungfuApi.Watcher | null,
  isShowOrigin = false,
): Record<string, string | number | bigint> => {
  const itemResolved = { ...item } as Record<string, string | number | bigint>;
  const instrument_type =
    'instrument_type' in item
      ? item.instrument_type
      : InstrumentTypeEnum.unknown;
  const isInstrumnetShotable = isShotable(instrument_type);

  if ('order_id' in item) {
    itemResolved.order_id = item.order_id.toString();
  }

  if ('trade_id' in item) {
    itemResolved.trade_id = item.trade_id.toString();
  }

  if ('instrument_id' in item) {
    itemResolved.instrument_id = item.instrument_id.toString();
  }

  if ('trade_time' in item && !isShowOrigin) {
    itemResolved.trade_time = dealKfTime(item.trade_time, true);
  }
  if ('insert_time' in item && !isShowOrigin) {
    itemResolved.insert_time = dealKfTime(item.insert_time, true);
  }
  if ('update_time' in item && !isShowOrigin) {
    itemResolved.update_time = dealKfTime(item.update_time, true);
  }
  if ('direction' in item) {
    itemResolved.direction = dealDirection(item.direction).name;
  }
  if ('side' in item) {
    itemResolved.side = dealSide(item.side).name;
  }
  if ('offset' in item) {
    if (isInstrumnetShotable) {
      itemResolved.offset = dealOffset(item.offset).name;
    } else {
      delete itemResolved.offset;
    }
  }
  if ('status' in item) {
    itemResolved.status = dealOrderStatus(
      item.status,
      item.error_msg || '',
    ).name;
  }
  if ('price_type' in item) {
    itemResolved.price_type = dealPriceType(item.price_type).name;
  }

  if ('volume_condition' in item) {
    if (isInstrumnetShotable) {
      itemResolved.volume_condition = dealVolumeCondition(
        item.volume_condition,
      ).name;
    } else {
      delete itemResolved.volume_condition;
    }
  }

  if ('time_condition' in item) {
    if (isInstrumnetShotable) {
      itemResolved.time_condition = dealTimeCondition(item.time_condition).name;
    } else {
      delete itemResolved.time_condition;
    }
  }

  if ('instrument_type' in item) {
    itemResolved.instrument_type = dealInstrumentType(
      item.instrument_type,
    ).name;
  }
  if ('hedge_flag' in item) {
    if (isInstrumnetShotable) {
      itemResolved.hedge_flag = dealHedgeFlag(item.hedge_flag).name;
    } else {
      delete itemResolved.hedge_flag;
    }
  }
  if ('is_swap' in item) {
    if (isInstrumnetShotable) {
      itemResolved.is_swap = dealIsSwap(item.is_swap).name;
    } else {
      delete itemResolved.is_swap;
    }
  }
  if ('source' in item && 'dest' in item && watcher) {
    itemResolved.source = resolveAccountId(
      watcher,
      item.source,
      item.dest,
    ).name;
  }
  if ('dest' in item && watcher) {
    itemResolved.dest = resolveClientId(watcher, item.dest).name;
  }
  if ('holder_uid' in item && watcher) {
    itemResolved.holder_uid = dealLocationUID(watcher, item.holder_uid);
  }

  return itemResolved;
};

export const getKungfuDataByDateRange = (
  date: number | string,
  dateType = HistoryDateEnum.naturalDate, //0 natural date, 1 tradingDate
): Promise<KungfuApi.TradingData | Record<string, unknown>> => {
  const tradingDay = dayjs(date).format('YYYYMMDD');
  const from = dayjs(date).format('YYYY-MM-DD');
  const to = dayjs(date).add(1, 'day').format('YYYY-MM-DD');
  const yesFrom = dayjs(date).add(-1, 'day').format('YYYY-MM-DD');
  const fridayFrom = dayjs(date).add(-3, 'day').format('YYYY-MM-DD');
  const fridayTo = dayjs(date).add(-2, 'day').format('YYYY-MM-DD');
  const dataTypeForHistory: KungfuApi.TradingDataTypeName[] = [
    'Order',
    'Trade',
    'OrderStat',
    'Position',
    'Asset',
    'OrderInput',
  ];

  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      //by trading date
      if (dateType === HistoryDateEnum.tradingDate) {
        const kungfuDataToday = history.selectPeriod(from, to);
        const kungfuDataYesterday = history.selectPeriod(yesFrom, from);
        const kungfuDataFriday = history.selectPeriod(fridayFrom, fridayTo);
        const historyData: KungfuApi.TradingData | Record<string, unknown> = {};

        if (!kungfuDataToday || !kungfuDataYesterday || !kungfuDataFriday)
          return reject(new Error('database_locked'));

        dataTypeForHistory.forEach((key) => {
          if (key === 'Order' || key === 'Trade' || key === 'OrderInput') {
            historyData[key] = Object.assign(
              kungfuDataToday[key].filter('trading_day', tradingDay),
              kungfuDataYesterday[key].filter('trading_day', tradingDay),
              kungfuDataFriday[key].filter('trading_day', tradingDay),
            );
          } else {
            historyData[key] = Object.assign(
              kungfuDataFriday[key as keyof KungfuApi.TradingData],
              kungfuDataYesterday[key as keyof KungfuApi.TradingData],
              kungfuDataToday[key as keyof KungfuApi.TradingData],
            );
          }
        });

        resolve(historyData);
      } else {
        const kungfuDataToday = history.selectPeriod(from, to);

        if (!kungfuDataToday) return reject(new Error('database_locked'));

        resolve(kungfuDataToday);
      }
      clearTimeout(timer);
    }, 160);
  });
};

export const getKungfuHistoryData = (
  date: string,
  dateType: HistoryDateEnum,
  tradingDataTypeName: KungfuApi.TradingDataTypeName | 'all',
  kfLocation?: KungfuApi.KfLocation,
): Promise<{
  tradingData: KungfuApi.TradingData;
}> => {
  return getKungfuDataByDateRange(date, dateType).then(
    (tradingData: KungfuApi.TradingData | Record<string, unknown>) => {
      if (tradingDataTypeName === 'all') {
        return {
          tradingData: tradingData as KungfuApi.TradingData,
        };
      }

      if (!kfLocation) {
        return {
          tradingData: tradingData as KungfuApi.TradingData,
        };
      }

      return {
        tradingData: tradingData as KungfuApi.TradingData,
      };
    },
  );
};

export const kfRequestMarketData = (
  watcher: KungfuApi.Watcher | null,
  exchangeId: string,
  instrumentId: string,
  mdLocation: KungfuApi.KfLocation,
): Promise<boolean> => {
  if (!watcher) {
    return Promise.reject(new Error('Watcher is NULL'));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  if (!watcher.isReadyToInteract(mdLocation)) {
    const sourceId = getIdByKfLocation(mdLocation);
    return Promise.reject(new Error(`Md ${sourceId} not ready`));
  }

  return Promise.resolve(
    watcher.requestMarketData(mdLocation, exchangeId, instrumentId),
  );
};

export const kfCancelOrder = (
  watcher: KungfuApi.Watcher | null,
  order: KungfuApi.Order,
): Promise<bigint> => {
  if (!watcher) {
    return Promise.reject(new Error(`Watcher is NULL`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  const { order_id, dest, source } = order;
  const sourceLocation = watcher.getLocation(source);
  const destLocation = watcher.getLocation(dest);

  if (!watcher.isReadyToInteract(sourceLocation)) {
    const accountId = getIdByKfLocation(sourceLocation);
    return Promise.reject(new Error(`Td ${accountId} not ready`));
  }

  const orderAction: KungfuApi.OrderAction = {
    ...longfist.OrderAction(),
    order_id,
  };

  if (!destLocation) {
    return Promise.resolve(watcher.cancelOrder(orderAction, sourceLocation));
  }

  return Promise.resolve(
    watcher.cancelOrder(orderAction, sourceLocation, destLocation),
  );
};

export const kfCancelAllOrders = (
  watcher: KungfuApi.Watcher | null,
  orders: KungfuApi.Order[],
): Promise<bigint[]> => {
  if (!watcher) {
    return Promise.reject(new Error(`Watcher is NULL`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  const cancelOrderTasks = orders.map(
    (item: KungfuApi.Order): Promise<bigint> => {
      return kfCancelOrder(watcher, item);
    },
  );

  return Promise.all(cancelOrderTasks);
};

export const kfMakeOrder = (
  watcher: KungfuApi.Watcher | null,
  makeOrderInput: KungfuApi.MakeOrderInput,
  tdLocation: KungfuApi.KfLocation,
  strategyLocation?: KungfuApi.KfLocation,
): Promise<bigint> => {
  if (!watcher) {
    return Promise.reject(new Error('Watcher is NULL'));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  if (!watcher.isReadyToInteract(tdLocation)) {
    const accountId = getIdByKfLocation(tdLocation);
    return Promise.reject(new Error(`Td ${accountId} not ready`));
  }

  const now = watcher.now();
  const orderInput: KungfuApi.OrderInput = {
    ...longfist.OrderInput(),
    ...makeOrderInput,
    block_id: BigInt(0),
    limit_price: makeOrderInput.limit_price || 0,
    frozen_price: makeOrderInput.limit_price || 0,
    volume: BigInt(makeOrderInput.volume),
    insert_time: now,
  };

  if (strategyLocation) {
    //设置orderInput的parentid, 来标记该order为策略手动下单
    return Promise.resolve(
      watcher.issueOrder(orderInput, tdLocation, strategyLocation),
    );
  } else {
    return Promise.resolve(watcher.issueOrder(orderInput, tdLocation));
  }
};

export const kfMakeBlockOrder = async (
  watcher: KungfuApi.Watcher | null,
  blockMessage: KungfuApi.BlockMessage,
  makeOrderInput: KungfuApi.MakeOrderInput,
  tdLocation: KungfuApi.KfLocation,
  strategyLocation?: KungfuApi.KfLocation,
): Promise<bigint> => {
  if (!watcher) {
    return Promise.reject(new Error('Watcher is NULL'));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  if (!watcher.isReadyToInteract(tdLocation)) {
    const accountId = getIdByKfLocation(tdLocation);
    return Promise.reject(new Error(`Td ${accountId} not ready`));
  }

  let block_id;
  if (blockMessage) {
    blockMessage = {
      ...blockMessage,
      is_specific: !!blockMessage.is_specific,
      opponent_seat: +blockMessage.opponent_seat,
      match_number: BigInt(blockMessage.match_number),
      insert_time: watcher.now(),
    };
    block_id = await watcher.issueBlockMessage(blockMessage, tdLocation);
  }
  if (!block_id) {
    return Promise.reject(new Error('Get block_id failed'));
  }

  const now = watcher.now();
  const orderInput: KungfuApi.OrderInput = {
    ...longfist.OrderInput(),
    ...makeOrderInput,
    block_id,
    limit_price: makeOrderInput.limit_price || 0,
    frozen_price: makeOrderInput.limit_price || 0,
    volume: BigInt(makeOrderInput.volume),
    insert_time: now,
  };

  if (strategyLocation) {
    //设置orderInput的parentid, 来标记该order为策略手动下单
    return Promise.resolve(
      watcher.issueOrder(orderInput, tdLocation, strategyLocation),
    );
  } else {
    return Promise.resolve(watcher.issueOrder(orderInput, tdLocation));
  }
};

export const makeOrderByOrderInput = (
  watcher: KungfuApi.Watcher | null,
  orderInput: KungfuApi.MakeOrderInput,
  kfLocation: KungfuApi.KfLocation,
  accountId: string,
): Promise<bigint> => {
  return new Promise((resolve, reject) => {
    if (!watcher) {
      reject(new Error(`Watcher is NULL`));
      return;
    }

    if (kfLocation.category === 'td') {
      return kfMakeOrder(watcher, orderInput, kfLocation)
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    } else if (kfLocation.category === 'strategy') {
      const tdLocation = getMdTdKfLocationByProcessId(`td_${accountId || ''}`);
      if (!tdLocation) {
        reject(new Error('下单账户信息错误'));
        return;
      }
      return kfMakeOrder(watcher, orderInput, tdLocation, kfLocation)
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    } else {
      const tdLocation = getMdTdKfLocationByProcessId(`td_${accountId || ''}`);
      if (!tdLocation) {
        reject(new Error('下单账户信息错误'));
        return;
      }
      return kfMakeOrder(watcher, orderInput, tdLocation)
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    }
  });
};

export const makeOrderByBlockMessage = (
  watcher: KungfuApi.Watcher | null,
  blockMessage: KungfuApi.BlockMessage,
  orderInput: KungfuApi.MakeOrderInput,
  kfLocation: KungfuApi.KfLocation,
  accountId: string,
): Promise<bigint> => {
  return new Promise((resolve, reject) => {
    if (!watcher) {
      reject(new Error(`Watcher is NULL`));
      return;
    }

    if (kfLocation.category === 'td') {
      return kfMakeBlockOrder(watcher, blockMessage, orderInput, kfLocation)
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    } else if (kfLocation.category === 'strategy') {
      const tdLocation = getMdTdKfLocationByProcessId(`td_${accountId || ''}`);
      if (!tdLocation) {
        reject(new Error('下单账户信息错误'));
        return;
      }
      return kfMakeBlockOrder(
        watcher,
        blockMessage,
        orderInput,
        tdLocation,
        kfLocation,
      )
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    } else {
      const tdLocation = getMdTdKfLocationByProcessId(`td_${accountId || ''}`);
      if (!tdLocation) {
        reject(new Error('下单账户信息错误'));
        return;
      }
      return kfMakeBlockOrder(watcher, blockMessage, orderInput, tdLocation)
        .then((order_id) => {
          resolve(order_id);
        })
        .catch((err) => {
          reject(err);
        });
    }
  });
};

export const getBasketInstrumentVolume = (
  volumeType: BasketVolumeTypeEnum,
  volume: bigint,
  rate: number,
  totalVolume: bigint,
) => {
  if (volumeType === BasketVolumeTypeEnum.Quantity) {
    return Number(volume);
  } else if (volumeType === BasketVolumeTypeEnum.Proportion) {
    return Math.floor((Number(totalVolume) * rate) / 100);
  }

  return 0;
};

export const makeOrderByBasketTrade = (
  watcher: KungfuApi.Watcher | null,
  basket: KungfuApi.Basket,
  basketOrderInput: KungfuApi.BasketOrderInput,
  basketInstruments: KungfuApi.BasketInstrument[],
  kfLocation: KungfuApi.KfLocation,
) => {
  if (!watcher) {
    return Promise.reject(new Error(`Watcher is NULL`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Watcher is not live`));
  }

  if (!watcher.isReadyToInteract(kfLocation)) {
    const accountId = getIdByKfLocation(kfLocation);
    return Promise.reject(new Error(`Td ${accountId} not ready`));
  }

  console.log(watcher, basket, basketInstruments, kfLocation);

  const now = watcher.now();
  const basketOrder: KungfuApi.BasketOrder = {
    ...longfist.BasketOrder(),
    insert_time: now,
    volume: BigInt(basket.total_volume),
    price_type: +basketOrderInput.price_type,
    price_level: +basketOrderInput.price_level,
    price_offset: +basketOrderInput.price_offset,
  };

  const parent_id = watcher.issueBasketOrder(basketOrder, kfLocation);

  const makeOrderTasks = basketInstruments.map((baksetInstrument) => {
    const volume = getBasketInstrumentVolume(
      basket.volume_type,
      baksetInstrument.volume,
      baksetInstrument.rate,
      basket.total_volume,
    );

    if (!volume) return Promise.resolve(0n);

    const orderInput: KungfuApi.OrderInput = {
      ...longfist.OrderInput(),
      parent_id,
      insert_time: now,
      instrument_id: `${baksetInstrument.instrument_id}`,
      exchange_id: `${baksetInstrument.exchange_id}`,
      instrument_type: +baksetInstrument.instrument_type,
      side: +basketOrderInput.side,
      offset: +basketOrderInput.offset,
      price_type: +basketOrderInput.price_type,
      limit_price: 0,
      frozen_price: 0,
      volume: BigInt(volume),
    };
    return Promise.resolve(watcher.issueOrder(orderInput, kfLocation));
  });

  return Promise.all(makeOrderTasks);
};

export const hashInstrumentUKey = (
  instrumentId: string,
  exchangeId: string,
): string => {
  return (BigInt(kf.hash(instrumentId)) ^ BigInt(kf.hash(exchangeId)))
    .toString(16)
    .padStart(16, '0');
};

export const dealOrder = (
  watcher: KungfuApi.Watcher,
  order: KungfuApi.Order,
  orderStats: KungfuApi.DataTable<KungfuApi.OrderStat>,
  isHistory = false,
): KungfuApi.OrderResolved => {
  const sourceResolvedData = resolveAccountId(
    watcher,
    order.source,
    order.dest,
  );
  const destResolvedData = resolveClientId(watcher, order.dest);
  const latencyData = dealOrderStat(orderStats, order.uid_key) || {
    latencySystem: '--',
    latencyNetwork: '--',
    avg_price: 0,
  };
  const statusData = dealOrderStatus(order.status, order.error_msg);
  return {
    ...order,
    source: order.source,
    dest: order.dest,
    uid_key: order.uid_key,
    source_resolved_data: sourceResolvedData,
    dest_resolved_data: destResolvedData,
    source_uname: sourceResolvedData.name,
    dest_uname: destResolvedData.name,
    status_uname: statusData.name,
    status_color: statusData.color || 'default',
    update_time_resolved: dealKfTime(order.update_time, isHistory),
    latency_system: latencyData.latencySystem,
    latency_network: latencyData.latencyNetwork,
    avg_price: latencyData.avg_price,
  };
};

export const dealTrade = (
  watcher: KungfuApi.Watcher,
  trade: KungfuApi.Trade,
  orderStats: KungfuApi.DataTable<KungfuApi.OrderStat>,
  isHistory = false,
): KungfuApi.TradeResolved => {
  const sourceResolvedData = resolveAccountId(
    watcher,
    trade.source,
    trade.dest,
  );
  const destResolvedData = resolveClientId(watcher, trade.dest);
  const orderUKey = trade.order_id.toString(16).padStart(16, '0');
  const latencyData = dealOrderStat(orderStats, orderUKey) || {
    latencyTrade: '--',
    trade_time: BigInt(0),
  };
  return {
    ...trade,
    source: trade.source,
    dest: trade.dest,
    uid_key: trade.uid_key,
    source_resolved_data: sourceResolvedData,
    dest_resolved_data: destResolvedData,
    source_uname: sourceResolvedData.name,
    dest_uname: destResolvedData.name,
    trade_time_resolved: dealKfTime(trade.trade_time, isHistory),
    kf_time_resovlved: dealKfTime(latencyData.trade_time, isHistory),
    latency_trade: latencyData.latencyTrade,
  };
};

export const getPosClosableVolume = (position: KungfuApi.Position): bigint => {
  return isShotable(position.instrument_type) ||
    isT0(position.instrument_type, position.exchange_id)
    ? BigInt(Math.max(+Number(position.volume - position.frozen_total), 0))
    : BigInt(
        Math.max(+Number(position.yesterday_volume - position.frozen_total), 0),
      );
};

export const dealPosition = (
  watcher: KungfuApi.Watcher,
  pos: KungfuApi.Position,
): KungfuApi.PositionResolved => {
  const holderLocation = watcher.getLocation(pos.holder_uid);
  const account_id_resolved =
    pos.ledger_category === LedgerCategoryEnum.td
      ? `${holderLocation.group}_${holderLocation.name}`
      : '--';
  const closable_volume = getPosClosableVolume(pos);
  return {
    ...pos,
    closable_volume,
    uid_key: pos.uid_key,
    account_id_resolved,
    instrument_id_resolved: `${pos.instrument_id} ${
      ExchangeIds[pos.exchange_id]?.name ?? ''
    }`,
  };
};
