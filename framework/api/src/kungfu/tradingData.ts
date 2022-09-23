import { Subject } from 'rxjs';
import { startWatcherSyncTask, startWatcher } from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = (stepInterval = 2000) => {
  startWatcher();
  startWatcherSyncTask(stepInterval, (watcher) => {
    tradingDataSubject.next(watcher);
  });
};
