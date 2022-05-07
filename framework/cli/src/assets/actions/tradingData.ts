import { getProcessIdByKfLocation } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  sendDataToProcessIdByPm2,
  Pm2Packet,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { filter, map, Observable } from 'rxjs';
import { globalState } from './globalState';

export const getOrders = (kfLocation: KungfuApi.KfLocation) => {
  if (!globalState.DZXY_PM_ID) {
    return Promise.resolve();
  }

  const { category, group, name } = kfLocation;
  return sendDataToProcessIdByPm2('GET_ORDERS', globalState.DZXY_PM_ID, {
    category,
    group,
    name,
  });
};

export const getTrades = (kfLocation: KungfuApi.KfLocation) => {
  if (!globalState.DZXY_PM_ID) {
    return Promise.resolve();
  }

  const { category, group, name } = kfLocation;
  return sendDataToProcessIdByPm2('GET_TRADES', globalState.DZXY_PM_ID, {
    category,
    group,
    name,
  });
};

export const getPosition = (kfLocation: KungfuApi.KfLocation) => {
  if (!globalState.DZXY_PM_ID) {
    return Promise.resolve();
  }

  const { category, group, name } = kfLocation;
  return sendDataToProcessIdByPm2('GET_POSITIONS', globalState.DZXY_PM_ID, {
    category,
    group,
    name,
  });
};

export const getAsset = (kfLocation: KungfuApi.KfLocation) => {
  if (!globalState.DZXY_PM_ID) {
    return Promise.resolve();
  }

  const { category, group, name } = kfLocation;
  return sendDataToProcessIdByPm2('GET_ASSETS', globalState.DZXY_PM_ID, {
    category,
    group,
    name,
  });
};

export const reqCancelAllOrders = (kfLocation: KungfuApi.KfLocation) => {
  if (!globalState.DZXY_PM_ID) {
    throw new Error('DZXY process not ready');
  }

  const { category, group, name } = kfLocation;
  return sendDataToProcessIdByPm2('CANCEL_ALL_ORDERS', globalState.DZXY_PM_ID, {
    category,
    group,
    name,
  });
};

export const exportTradingData = (date, output_folder) => {
  if (!globalState.DZXY_PM_ID) {
    throw new Error('DZXY process not ready');
  }

  return sendDataToProcessIdByPm2('EXPORT', globalState.DZXY_PM_ID, {
    date,
    output_folder,
  });
};

export const ordersObservable = (
  kfLocation: KungfuApi.KfLocation,
): Observable<KungfuApi.OrderResolved[]> => {
  const processId = getProcessIdByKfLocation(kfLocation);
  return globalState.DZXY_SUBJECT.pipe(
    filter((packet: Pm2Packet) => {
      if (packet.data.type === 'ORDER') {
        if (packet.data.body.processId === processId) {
          return true;
        }
      }
      return false;
    }),
    map((packet: Pm2Packet) => {
      return packet.data.body.orders as unknown as KungfuApi.OrderResolved[];
    }),
  );
};

export const tradesObservable = (
  kfLocation: KungfuApi.KfLocation,
): Observable<KungfuApi.TradeResolved[]> => {
  const processId = getProcessIdByKfLocation(kfLocation);
  return globalState.DZXY_SUBJECT.pipe(
    filter((packet: Pm2Packet) => {
      if (packet.data.type === 'TRADE') {
        if (packet.data.body.processId === processId) {
          return true;
        }
      }
      return false;
    }),
    map((packet: Pm2Packet) => {
      return packet.data.body.trades as unknown as KungfuApi.TradeResolved[];
    }),
  );
};

export const posObservable = (
  kfLocation: KungfuApi.KfLocation,
): Observable<KungfuApi.PositionResolved[]> => {
  const processId = getProcessIdByKfLocation(kfLocation);
  return globalState.DZXY_SUBJECT.pipe(
    filter((packet: Pm2Packet) => {
      if (packet.data.type === 'POSITION') {
        if (packet.data.body.processId === processId) {
          return true;
        }
      }
      return false;
    }),
    map((packet: Pm2Packet) => {
      return packet.data.body
        .position as unknown as KungfuApi.PositionResolved[];
    }),
  );
};

export const assetObservable = (
  kfLocation: KungfuApi.KfLocation,
): Observable<KungfuApi.Asset> => {
  const processId = getProcessIdByKfLocation(kfLocation);
  return globalState.DZXY_SUBJECT.pipe(
    filter((packet: Pm2Packet) => {
      if (packet.data.type === 'ASSET') {
        if (packet.data.body.processId === processId) {
          return true;
        }
      }
      return false;
    }),
    map((packet: Pm2Packet) => {
      return packet.data.body.asset as unknown as KungfuApi.Asset;
    }),
  );
};
