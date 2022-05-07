import './assets/methods/setDzxyEnv';
import path from 'path';
import { triggerStartStep } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import {
  dealTradingData,
  getOrderTradeFilterKey,
  getProcessIdByKfLocation,
  setTimerPromiseTask,
  switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { SwitchKfLocationPacketData } from './typings';
import {
  dealAppStates,
  dealAssetsByHolderUID,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
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

triggerStartStep();

setTimerPromiseTask((): Promise<void> => {
  return new Promise((resolve) => {
    process.send &&
      process.send({
        type: 'process:msg',
        data: {
          type: 'APP_STATES',
          body: dealAppStates(watcher, watcher?.appStates || {}),
        },
      });

    process.send &&
      process.send({
        type: 'process:msg',
        data: {
          type: 'WATCHER_IS_LIVE',
          body: watcher ? !!watcher.isLive() : false,
        },
      });

    resolve();
  });
}, 3000);

process.on('message', (packet: Pm2PacketMain) => {
  const { type, topic, data } = packet;
  if (type !== 'process:msg') {
    return;
  }

  switch (topic) {
    case 'SWITCH_KF_LOCATION':
      swithKfLocationResolved(data as SwitchKfLocationPacketData);
      break;
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
    throw new Error('watcher is null');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const orders = dealTradingData(watcher, watcher.ledger, 'Order', kfLocation)
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
    throw new Error('watcher is null');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const trades = dealTradingData(watcher, watcher.ledger, 'Trade', kfLocation)
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
    throw new Error('watcher is null');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const position = dealTradingData(
    watcher,
    watcher.ledger,
    'Position',
    kfLocation,
  ).map((item) => dealPosition(item as KungfuApi.Position));

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
    throw new Error('watcher is null');
  }

  const kfLocation = fromPacketToKfLocation(packet);
  const assets = dealAssetsByHolderUID(watcher, watcher.ledger.Asset);
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

function swithKfLocationResolved(data: SwitchKfLocationPacketData) {
  const { category, group, name, value, status } = data;
  const kfConfig: KungfuApi.KfConfig = {
    category,
    group,
    name,
    value: value,
    location_uid: 0,
    mode: 'live',
  };

  return switchKfLocation(watcher, kfConfig, !status).catch((err) => {
    console.error(err.message);
  });
}

function cancellAllOrders(packet: Pm2PacketMain) {
  if (!watcher) {
    throw new Error('watcher is null');
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
    watcher,
    date,
    HistoryDateEnum.naturalDate,
    'all',
  );

  const orders = tradingData.Order.sort('update_time');
  const trades = tradingData.Trade.sort('trade_time');
  const orderStat = tradingData.OrderStat.sort('insert_time');
  const positions = tradingData.Trade.list();

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
