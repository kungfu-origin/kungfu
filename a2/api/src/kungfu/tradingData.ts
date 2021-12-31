import { Subject } from 'rxjs';
import {
    startGetKungfuWatcherStep,
    startUpdateKungfuWatcherQuotes,
} from './watcher';
export const tradingDataSubject = new Subject<KungfuApi.Watcher>();

startGetKungfuWatcherStep(1000, (watcher) => {
    tradingDataSubject.next(watcher);
});

startUpdateKungfuWatcherQuotes(2000);
