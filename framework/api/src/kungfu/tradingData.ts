import { Subject } from 'rxjs';
import { startWatcherSyncTask, startWatcher } from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = (stepInterval = 2000) => {
  startWatcher();
  return startWatcherSyncTask(stepInterval, (watcher) => {
    console.log('triggerStartStep');
    tradingDataSubject.next(watcher);
  });
};
