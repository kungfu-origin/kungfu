import './assets/methods/setDzxyEnv';
import {
  tradingDataSubject,
  triggerStartStep,
} from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import {
  setTimerPromiseTask,
  switchKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Pm2PacketMain, SwitchKfLocationPacketData } from './typings';
import { dealAppStates } from '@kungfu-trader/kungfu-js-api/kungfu/watcher';

triggerStartStep();

let WATCHER: KungfuApi.Watcher | null = null;

tradingDataSubject.subscribe((watcher) => {
  WATCHER = watcher;
});

setTimerPromiseTask((): Promise<void> => {
  return new Promise((resolve) => {
    process.send({
      type: 'process:msg',
      data: {
        type: 'APP_STATES',
        body: dealAppStates(WATCHER?.appStates || {}),
      },
    });

    process.send({
      type: 'process:msg',
      data: {
        type: 'WATCHER_IS_LIVE',
        body: WATCHER ? !!WATCHER.isLive() : false,
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
  }
});

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

  return switchKfLocation(WATCHER, kfConfig, !status);
}
