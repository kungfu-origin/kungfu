import { Subject } from 'rxjs';
import { startWatcherSyncTask, startWatcher } from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = () => {
  startWatcher();
  startWatcherSyncTask(2000, (watcher) => {
    tradingDataSubject.next(watcher);
  });
};
