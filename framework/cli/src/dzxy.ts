import './setDzxyEnv';
import './injectGlobal';
import path from 'path';
import { triggerStartStep } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import {
  getOrderTradeFilterKey,
  getProcessIdByKfLocation,
  getTradingDataSortKey,
  setTimerPromiseTask,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { dealAssetsByHolderUID } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Pm2PacketMain } from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import {
  dealOrder,
  dealPosition,
  dealTrade,
  dealTradingDataItem,
  getKungfuHistoryData,
  kfCancelAllOrders,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';
import { HistoryDateEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { writeCSV } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
import { Row } from '@fast-csv/format';
import { useAllExtComponentByPosition } from './assets/methods/utils';

triggerStartStep();
useAllExtComponentByPosition('dzxy');

setTimerPromiseTask((): Promise<void> => {
  return new Promise((resolve) => {
    console.log('Heartbeat watcher living', watcher?.isLive());
    process.send &&
      process.send({
        type: 'process:msg',
        data: {
          type: 'WATCHER_IS_LIVE',
          body: watcher ? watcher.isLive() && watcher.isStarted() : false,
        },
      });

    resolve();
  });
}, 2000);

process.on('message', (packet: Pm2PacketMain) => {
  const { type, topic } = packet;
  if (type !== 'process:msg') {
    return;
  }

  console.log('dzxy get msg topic', topic);

  switch (topic) {
    case 'GET_ORDERS':
      resOrders(packet);
      break;
    case 'GET_TRADES':
      resTrades(packet);
      break;
    case 'GET_POSITIONS':
      resPosition(packet);
      break;
    case 'GET_ASSETS':
      resAsset(packet);
      break;
    case 'CANCEL_ALL_ORDERS':
      cancellAllOrders(packet);
      break;
    case 'EXPORT':
      dealExport(packet);
      break;
  }
});

function resOrders(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const orders = globalThis.HookKeeper.getHooks()
    .dealTradingData.trigger(watcher, kfLocation, watcher.ledger.Order, 'order')
    .slice(0, 10)
    .map((item) =>
      dealOrder(
        watcher as KungfuApi.Watcher,
        item as KungfuApi.Order,
        (watcher as KungfuApi.Watcher).ledger.OrderStat,
      ),
    );

  turnBigIntToString(orders);

  process.send &&
    process.send({
      type: 'process:msg',
      data: {
        type: 'ORDER',
        body: {
          processId: getProcessIdByKfLocation(kfLocation),
          orders,
        },
      },
    });
}

function resTrades(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const trades = globalThis.HookKeeper.getHooks()
    .dealTradingData.trigger(watcher, kfLocation, watcher.ledger.Trade, 'trade')
    .slice(0, 10)
    .map((item) =>
      dealTrade(
        watcher as KungfuApi.Watcher,
        item as KungfuApi.Trade,
        (watcher as KungfuApi.Watcher).ledger.OrderStat,
      ),
    );

  turnBigIntToString(trades);

  process.send &&
    process.send({
      type: 'process:msg',
      data: {
        type: 'TRADE',
        body: {
          processId: getProcessIdByKfLocation(kfLocation),
          trades,
        },
      },
    });
}

function resPosition(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const position = globalThis.HookKeeper.getHooks()
    .dealTradingData.trigger(
      watcher,
      kfLocation,
      watcher.ledger.Position,
      'position',
    )
    .map((item) =>
      dealPosition(watcher as KungfuApi.Watcher, item as KungfuApi.Position),
    );

  turnBigIntToString(position);

  process.send &&
    process.send({
      type: 'process:msg',
      data: {
        type: 'POSITION',
        body: {
          processId: getProcessIdByKfLocation(kfLocation),
          position,
        },
      },
    });
}

function resAsset(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const assets = dealAssetsByHolderUID<KungfuApi.Asset>(
    watcher,
    watcher.ledger.Asset,
  );
  const processId = getProcessIdByKfLocation(kfLocation);
  const assetsResolved = [assets[processId] || {}];
  turnBigIntToString(assetsResolved);

  process.send &&
    process.send({
      type: 'process:msg',
      data: {
        type: 'ASSET',
        body: {
          processId,
          asset: assetsResolved[0],
        },
      },
    });
}

function cancellAllOrders(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  const { category } = packet.data as KungfuApi.KfLocation;
  const kfLocation = fromPacketToKfLocation(packet);
  const filterKey = getOrderTradeFilterKey(category);
  const orders = watcher.ledger.Order.filter(
    filterKey,
    watcher.getLocationUID(kfLocation),
  ).list();
  return kfCancelAllOrders(watcher, orders).catch((err) => {
    console.error(err.message);
  });
}

function dealExport(packet: Pm2PacketMain) {
  const { date, output_folder } = packet.data as {
    date: string;
    output_folder: string;
  };
  return exportTradingData(date, output_folder);
}

function dealTradingDataItemResolved(item: KungfuApi.TradingDataTypes): Row {
  return dealTradingDataItem(item, watcher) as Row;
}

async function exportTradingData(date, output_folder) {
  const { tradingData } = await getKungfuHistoryData(
    date,
    HistoryDateEnum.naturalDate,
    'all',
  );

  const orderSortKey = getTradingDataSortKey('Order');
  const tradeSortKey = getTradingDataSortKey('Trade');
  const orderStatSortKey = getTradingDataSortKey('OrderStat');
  const positionSortKey = getTradingDataSortKey('Position');
  const orders = tradingData.Order.sort(orderSortKey);
  const trades = tradingData.Trade.sort(tradeSortKey);
  const orderStat = tradingData.OrderStat.sort(orderStatSortKey);
  const positions = tradingData.Position.sort(positionSortKey);

  const ordersFilename = path.join(output_folder, `orders-${date}}`);
  const tradesFilename = path.join(output_folder, `trades-${date}`);
  const orderStatFilename = path.join(output_folder, `orderStats-${date}`);
  const posFilename = path.join(output_folder, `pos-${date}`);

  return Promise.all([
    writeCSV(ordersFilename, orders, dealTradingDataItemResolved),
    writeCSV(tradesFilename, trades, dealTradingDataItemResolved),
    writeCSV(orderStatFilename, orderStat, dealTradingDataItemResolved),
    writeCSV(posFilename, positions, dealTradingDataItemResolved),
  ]);
}

function fromPacketToKfLocation(packet: Pm2PacketMain): KungfuApi.KfLocation {
  const { category, group, name } = packet.data as KungfuApi.KfLocation;
  return {
    category,
    group,
    name,
    mode: 'live',
  };
}

function turnBigIntToString<T>(list: T[]) {
  list.forEach((item) => {
    Object.keys(item).forEach((key) => {
      if (typeof item[key] === 'bigint') {
        item[key] = Number(item[key]).toFixed(0);
      }
    });
  });
}
