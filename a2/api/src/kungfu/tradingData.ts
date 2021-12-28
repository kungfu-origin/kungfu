import { Subject } from 'rxjs';
import {
    startGetKungfuWatcherStep,
    startUpdateKungfuWatcherQuotes,
} from './watcher';
export const tradingDataSubject = new Subject<Watcher>();

startGetKungfuWatcherStep(1000, (watcher) => {
    tradingDataSubject.next(watcher);
});

startUpdateKungfuWatcherQuotes(2000);