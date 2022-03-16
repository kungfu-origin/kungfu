import { Subject } from 'rxjs';
import {
  startGetKungfuWatcherStep,
  startUpdateKungfuWatcherQuotes,
  startStep,
} from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

export const triggerStartStep = () => {
  startStep();
  startGetKungfuWatcherStep(2000, (watcher) => {
    tradingDataSubject.next(watcher);
  });

  // startUpdateKungfuWatcherQuotes(3000);
};
