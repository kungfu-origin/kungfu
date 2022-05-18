import dayjs from 'dayjs';
import { kungfu } from '@kungfu-trader/kungfu-core';
import { KF_RUNTIME_DIR } from '../config/pathConfig';
import {
  dealDirection,
  dealHedgeFlag,
  dealInstrumentType,
  dealLocationUID,
  dealOffset,
  dealOrderStat,
  dealOrderStatus,
  dealPriceType,
  dealSide,
  dealTimeCondition,
  dealTradingData,
  dealVolumeCondition,
  getIdByKfLocation,
  getMdTdKfLocationByProcessId,
  kfLogger,
  resolveAccountId,
  resolveClientId,
} from '../utils/busiUtils';
import { HistoryDateEnum, MakeOrderByWatcherEnum } from '../typings/enums';
import { ExchangeIds } from '../config/tradingConfig';

if (process.env.RENDERER_TYPE === 'logview') {
  throw new Error('Logview should not use kungfu.node');
}

export const kf = kungfu();

kfLogger.info('Load kungfu node');

export const configStore = kf.ConfigStore(KF_RUNTIME_DIR);
export const riskSettingStore = kf.RiskSettingStore(KF_RUNTIME_DIR);
export const history = kf.History(KF_RUNTIME_DIR);
export const commissionStore = kf.CommissionStore(KF_RUNTIME_DIR);
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
  isShowOrigin: boolean = false,
): Record<string, string | number | bigint> => {
  const itemResolved = { ...item } as Record<string, string | number | bigint>;
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
    itemResolved.offset = dealOffset(item.offset).name;
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
    itemResolved.volume_condition = dealVolumeCondition(
      item.volume_condition,
    ).name;
  }

  if ('time_condition' in item) {
    itemResolved.time_condition = dealTimeCondition(item.time_condition).name;
  }

  if ('instrument_type' in item) {
    itemResolved.instrument_type = dealInstrumentType(
      item.instrument_type,
    ).name;
  }
  if ('hedge_flag' in item) {
    itemResolved.hedge_flag = dealHedgeFlag(item.hedge_flag).name;
  }
  if ('source' in item && watcher && 'parent_id' in item) {
    itemResolved.source = resolveAccountId(
      watcher,
      item.source,
      item.dest,
      item.parent_id,
    ).name;
  }
  if ('dest' in item && watcher && 'parent_id' in item) {
    itemResolved.dest = resolveClientId(
      watcher,
      item.dest,
      item.parent_id,
    ).name;
  }
  if ('source' in item && watcher && 'parent_order_id' in item) {
    itemResolved.source = resolveAccountId(
      watcher,
      item.source,
      item.dest,
      item.parent_order_id,
    ).name;
  }
  if ('dest' in item && watcher && 'parent_order_id' in item) {
    itemResolved.dest = resolveClientId(
      watcher,
      item.dest,
      item.parent_order_id,
    ).name;
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

  return new Promise((resolve) => {
    const timer = setTimeout(() => {
      //by trading date
      if (dateType === HistoryDateEnum.tradingDate) {
        const kungfuDataToday = history.selectPeriod(from, to);
        const kungfuDataYesterday = history.selectPeriod(yesFrom, from);
        const kungfuDataFriday = history.selectPeriod(fridayFrom, fridayTo);
        const historyData: KungfuApi.TradingData | Record<string, unknown> = {};
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
        resolve(kungfuDataToday);
      }
      clearTimeout(timer);
    }, 160);
  });
};

export const getKungfuHistoryData = (
  watcher: KungfuApi.Watcher | null,
  date: string,
  dateType: HistoryDateEnum,
  tradingDataTypeName: KungfuApi.TradingDataTypeName | 'all',
  kfLocation?: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<{
  tradingData: KungfuApi.TradingData;
  historyDatas: KungfuApi.TradingDataTypes[];
}> => {
  return getKungfuDataByDateRange(date, dateType).then(
    (tradingData: KungfuApi.TradingData | Record<string, unknown>) => {
      if (tradingDataTypeName === 'all') {
        return {
          tradingData: tradingData as KungfuApi.TradingData,
          historyDatas: [],
        };
      }

      if (!kfLocation) {
        return {
          tradingData: tradingData as KungfuApi.TradingData,
          historyDatas: [],
        };
      }

      return {
        tradingData: tradingData as KungfuApi.TradingData,
        historyDatas: dealTradingData(
          watcher,
          tradingData as KungfuApi.TradingData,
          tradingDataTypeName,
          kfLocation,
        ),
      };
    },
  );
};

export const kfRequestMarketData = (
  watcher: KungfuApi.Watcher | null,
  exchangeId: string,
  instrumentId: string,
  mdLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): Promise<void> => {
  if (!watcher) {
    return Promise.reject(new Error(`Watcher 错误`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Master 进程未连接`));
  }

  if (!watcher.isReadyToInteract(mdLocation)) {
    const sourceId = getIdByKfLocation(mdLocation);
    return Promise.reject(new Error(`行情源 ${sourceId} 未就绪`));
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
    return Promise.reject(new Error(`Watcher 错误`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Master 未连接`));
  }

  const { order_id, dest, source } = order;
  const sourceLocation = watcher.getLocation(source);
  const destLocation = watcher.getLocation(dest);

  if (!watcher.isReadyToInteract(sourceLocation)) {
    const accountId = getIdByKfLocation(sourceLocation);
    return Promise.reject(new Error(`交易账户 ${accountId} 未就绪`));
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
    return Promise.reject(new Error(`Watcher 错误`));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Master 未连接`));
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
    return Promise.reject(new Error('Watcher 错误'));
  }

  if (!watcher.isLive()) {
    return Promise.reject(new Error(`Master 未连接`));
  }

  if (!watcher.isReadyToInteract(tdLocation)) {
    const accountId = getIdByKfLocation(tdLocation);
    return Promise.reject(new Error(`交易账户 ${accountId} 未就绪`));
  }

  const now = watcher.now();
  const orderInput: KungfuApi.OrderInput = {
    ...longfist.OrderInput(),
    ...makeOrderInput,
    limit_price: makeOrderInput.limit_price || 0,
    frozen_price: makeOrderInput.limit_price || 0,
    volume: BigInt(makeOrderInput.volume),
    insert_time: now,
  };

  if (strategyLocation) {
    //设置orderInput的parentid, 来标记该order为策略手动下单
    return Promise.resolve(
      watcher.issueOrder(
        {
          ...orderInput,
          parent_id: BigInt(MakeOrderByWatcherEnum.Manual),
        },
        tdLocation,
        strategyLocation,
      ),
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
      reject(new Error(`Watcher 错误`));
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
    order.parent_id,
  );
  const destResolvedData = resolveClientId(
    watcher,
    order.dest,
    order.parent_id,
  );
  const latencyData = dealOrderStat(orderStats, order.uid_key) || {
    latencySystem: '--',
    latencyNetwork: '--',
  };
  return {
    ...order,
    source: order.source,
    dest: order.dest,
    uid_key: order.uid_key,
    source_resolved_data: sourceResolvedData,
    dest_resolved_data: destResolvedData,
    source_uname: sourceResolvedData.name,
    dest_uname: destResolvedData.name,
    update_time_resolved: dealKfTime(order.update_time, isHistory),
    latency_system: latencyData.latencySystem,
    latency_network: latencyData.latencyNetwork,
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
    trade.parent_order_id,
  );
  const destResolvedData = resolveClientId(
    watcher,
    trade.dest,
    trade.parent_order_id,
  );
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

export const dealPosition = (
  pos: KungfuApi.Position,
): KungfuApi.PositionResolved => {
  return {
    ...pos,
    uid_key: pos.uid_key,
    account_id_resolved: `${pos.source_id}_${pos.account_id}`,
    instrument_id_resolved: `${pos.instrument_id} ${
      ExchangeIds[pos.exchange_id].name
    }`,
  };
};
