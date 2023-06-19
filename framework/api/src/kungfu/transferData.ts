import { Subject } from 'rxjs';
import { startWatcherSyncTask, startWatcher } from './watcher';
export const transferDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = (stepInterval = 2000) => {
  startWatcher();
  startWatcherSyncTask(stepInterval, (watcher) => {
    transferDataSubject.next(watcher);
  });
};
