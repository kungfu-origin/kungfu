import { Subject } from 'rxjs';
import {
  startGetKungfuWatcherStep,
  startUpdateKungfuWatcherQuotes,
} from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = () => {
  startGetKungfuWatcherStep(2000, (watcher) => {
    tradingDataSubject.next(watcher);
  });

  startUpdateKungfuWatcherQuotes(3000);
};
