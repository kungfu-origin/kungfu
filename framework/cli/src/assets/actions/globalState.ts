import { Subject } from 'rxjs';
import { Pm2Bus, Pm2Packet } from 'src/typings';
import pm2 from 'pm2';

export const globalState = {
  DZXY_PM_ID: 0,
  DZXY_WATCHER_IS_LIVE: false,
  DZXY_SUBJECT: new Subject<Pm2Packet>(),
};

const timer = setTimeout(() => {
  globalState.DZXY_SUBJECT.next({
    process: {
      pm_id: -1,
    },
    data: {
      type: 'APP_STATES',
      body: {},
    },
  });
  clearTimeout(timer);
}, 1000);

pm2.launchBus((err: Error, pm2_bus: Pm2Bus) => {
  if (err) {
    console.error('pm2 launchBus Error', err);
    return;
  }

  pm2_bus.on('process:msg', (packet: Pm2Packet) => {
    const processData = packet.process;
    globalState.DZXY_PM_ID = processData.pm_id;

    if (packet.data.type === 'WATCHER_IS_LIVE') {
      globalState.DZXY_WATCHER_IS_LIVE = !!packet.data.body;
    }

    globalState.DZXY_SUBJECT.next(packet);
  });
});
