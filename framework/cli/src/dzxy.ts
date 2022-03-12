import './assets/methods/setDzxyEnv';
import { triggerStartStep } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import {
  dealTradingData,
  getOrderTradeFilterKey,
  getProcessIdByKfLocation,
  setTimerPromiseTask,
  switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Pm2PacketMain, SwitchKfLocationPacketData } from './typings';
import {
  dealAppStates,
  dealAssetsByHolderUID,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

import {
  dealOrder,
  dealPosition,
  dealTrade,
  kfCancelAllOrders,
} from '@kungfu-trader/kungfu-js-api/kungfu';
import { watcher } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';

triggerStartStep();

setTimerPromiseTask((): Promise<void> => {
  return new Promise((resolve) => {
    process.send({
      type: 'process:msg',
      data: {
        type: 'APP_STATES',
        body: dealAppStates(watcher, watcher?.appStates || {}),
      },
    });

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
  }
});

function resOrders(packet: Pm2PacketMain) {
  const kfLocation = fromPacketToKfLocation(packet);
  const orders = dealTradingData(watcher, watcher.ledger, 'Order', kfLocation)
    .slice(0, 10)
    .map((item) =>
      dealOrder(watcher, item as KungfuApi.Order, watcher.ledger.OrderStat),
    );

  turnBigIntToString(orders);

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
  const kfLocation = fromPacketToKfLocation(packet);
  const trades = dealTradingData(watcher, watcher.ledger, 'Trade', kfLocation)
    .slice(0, 10)
    .map((item) =>
      dealTrade(watcher, item as KungfuApi.Trade, watcher.ledger.OrderStat),
    );

  turnBigIntToString(trades);

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
  const kfLocation = fromPacketToKfLocation(packet);
  const position = dealTradingData(
    watcher,
    watcher.ledger,
    'Position',
    kfLocation,
  ).map((item) => dealPosition(watcher, item as KungfuApi.Position));

  turnBigIntToString(position);

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
  const kfLocation = fromPacketToKfLocation(packet);
  const assets = dealAssetsByHolderUID(watcher, watcher.ledger.Asset);
  const processId = getProcessIdByKfLocation(kfLocation);
  const assetsResolved = [assets[processId] || {}];
  turnBigIntToString(assetsResolved);

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

function cancellAllOrders(packet: Pm2PacketMain) {
  const { category, group, name } = packet.data as KungfuApi.KfLocation;
  const kfLocation: KungfuApi.KfLocation = {
    category,
    group,
    name,
    mode: 'live',
  };
  const filterKey = getOrderTradeFilterKey(category);
  const orders = watcher.ledger.Order.filter(
    filterKey,
    watcher.getLocationUID(kfLocation),
  ).list();
  return kfCancelAllOrders(watcher, orders, kfLocation).catch((err) => {
    console.error(err.message);
  });
}
