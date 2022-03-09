// import './assets/methods/setDzxyEnv';
// import {
//   tradingDataSubject,
//   triggerStartStep,
// } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
// import pm2 from 'pm2';
// // import { Pm2Bus, Pm2Packet } from './typings';

// triggerStartStep();

// let WATCHER: KungfuApi.Watcher | null = null;

// tradingDataSubject.subscribe((watcher) => {
//   WATCHER = watcher;
// });

setInterval(() => {
  console.log(111);
}, 1000);

process.on('SIGINT', () => {
  process.exit(0);
});
