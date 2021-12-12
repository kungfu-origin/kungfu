import { Subject } from 'rxjs';
import { startGetKungfuWatcherStep } from './watcher';
export const tradingDataSubject = new Subject<Watcher>();

startGetKungfuWatcherStep(1000, (watcher) => {
    tradingDataSubject.next(watcher);
});
