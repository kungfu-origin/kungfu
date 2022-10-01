import { Subject } from 'rxjs';
import {
  Pm2Packet,
  Pm2Bus,
  pm2LaunchBus,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import { BrokerStateStatusTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';

export const globalState = {
  DZXY_PM_ID: 0,
  DZXY_SUBJECT: new Subject<Pm2Packet>(),
  APP_STATES_SUBJECT: new Subject<Record<string, BrokerStateStatusTypes>>(),
};

if (process.env.APP_TYPE === 'cli') {
  pm2LaunchBus((err: Error, pm2_bus: Pm2Bus) => {
    if (err) {
      console.error('pm2 launchBus Error', err);
      return;
    }

    pm2_bus &&
      pm2_bus.on('process:msg', (packet: Pm2Packet) => {
        const processData = packet.process;
        globalState.DZXY_PM_ID = processData.pm_id;
        globalState.DZXY_SUBJECT.next(packet);
      });
  });
}
